#include "pch.h"
#include "ThisApplication.h"
#include "components\level\ScoreAdvanceRender.h"
#include "components\player\PlayerComponent.h"
#include "coreEntity\component\standard\LayerGroupComponent.h"
#include "coreEntity\domain\EntityDomain.h"
#include "coreEntity\domain\EntityDomainProvider.h"
#include "coreEntity\entity\Entity.h"
#include "entities\EntityEvents.h"
#include "Graph\2D\2dRenderer.h"
#include "Graph\2D\Sprite.h"
#include "Graph\2D\SpriteList.h"
#include "Graph\Anim\AnimPos.h"
#include "Graph\Font\SpriteFont.h"
#include "Module\ModuleFactory.h"
#include "services\EntityFactoryService.h"
#include "services\GlobalPlayerService.h"
#include "services\PlayerService.h"

static ff::ModuleStartup RegisterModuleClass([](ff::Module &module)
{
	ff::StaticString name(L"ScoreAdvanceRender");
	module.RegisterClassT<ScoreAdvanceRender>(name);
});

BEGIN_INTERFACES(ScoreAdvanceRender)
	HAS_INTERFACE(IAdvanceComponent)
	HAS_INTERFACE(I2dRenderComponent)
END_INTERFACES()

ScoreAdvanceRender::ScoreAdvanceRender()
	: _counter(0)
	, _highScore(0)
	, _coopLives(0)
{
	ff::ZeroObject(_lives);
}

ScoreAdvanceRender::~ScoreAdvanceRender()
{
}

HRESULT ScoreAdvanceRender::_Construct(IUnknown *unkOuter)
{
	ff::ComPtr<IEntityDomainProvider> pDomainProvider;
	assertRetVal(pDomainProvider.QueryFrom(unkOuter), E_INVALIDARG);

	IEntityDomain* pDomain = pDomainProvider->GetDomain();
	ThisApplication* pApp = ThisApplication::Get(pDomain);

	_playStartListener.Init(pDomain, ENTITY_EVENT_PLAYING_START, this);
	_font.Init(L"Classic");

	assertRetVal(GetService(pDomain, &_globalPlayers), E_FAIL);
	assertRetVal(GetService(pDomain, &_players), E_FAIL);

	ff::TypedResource<ff::ISpriteList> pSprites(L"Sprites");
	assertRetVal(ff::CreateSpriteResource(pSprites.GetResourceValue(), ff::String(L"Player Body Standing"), &_playerBody), E_FAIL);
	assertRetVal(ff::CreateSpriteResource(pSprites.GetResourceValue(), ff::String(L"Player Turret Standing"), &_playerTurret), E_FAIL);
	assertRetVal(ff::CreateSpriteResource(pSprites.GetResourceValue(), ff::String(L"Player Wheels Standing"), &_playerWheels), E_FAIL);

	// Get initial high score
	if (Globals::HasHighScores(_globalPlayers->GetDifficulty()))
	{
		bool isCoop = (_globalPlayers->GetGameMode() == GAME_MODE_COOP);
		size_t nHighScores = pApp->GetHighScores().GetHighScoreCount(isCoop, _globalPlayers->GetDifficulty());

		if (nHighScores)
		{
			_highScore = pApp->GetHighScores().GetHighScore(isCoop, _globalPlayers->GetDifficulty(), 0)._stats.GetScore();
		}
	}

	UpdateLivesCount();

	return __super::_Construct(unkOuter);
}

int ScoreAdvanceRender::GetAdvancePriority() const
{
	return LAYER_PRI_NORMAL;
}

void ScoreAdvanceRender::Advance(IEntity *entity)
{
	_counter++;

	bool isCoop = (_globalPlayers->GetGameMode() == GAME_MODE_COOP);
	size_t prevLives[2] = { _lives[0], _lives[1] };
	size_t prevCoopLives = _coopLives;

	UpdateLivesCount();

	// Lives
	for (size_t i = 0; i < _globalPlayers->GetPlayerCount(); i++)
	{
		if (i == 0 || !isCoop)
		{
			bool bActive = isCoop || _globalPlayers->GetCurrentPlayer(0) == i;
			size_t nLives = isCoop ? _coopLives : _lives[i];
			size_t nPrevLives = isCoop ? prevCoopLives : prevLives[i];

			if (bActive && nLives > nPrevLives)
			{
				ff::ComPtr<EntityFactoryService> pFactory;
				if (GetService(entity, &pFactory))
				{
					ff::PointFloat pos = Globals::GetPlayerLifePos(i, nLives - 1) + ff::PointFloat(0, -25);
					pFactory->CreateFreeLifeIndicator(entity->GetDomain(), pos);
				}
			}
		}
	}
}

int ScoreAdvanceRender::Get2dRenderPriority() const
{
	return LAYER_PRI_HIGH;
}

static const DirectX::XMFLOAT4 s_colorText(0.8706f, 0.8706f, 1, 1);

void ScoreAdvanceRender::Render(IEntity *entity, I2dLayerGroup *group, ff::I2dRenderer *render)
{
	size_t nHighScore = _highScore;
	bool isCoop = (_globalPlayers->GetGameMode() == GAME_MODE_COOP);
	ff::ISpriteFont *font = _font.GetObject();

	if (isCoop || _globalPlayers->GetPlayerCount() == 1)
	{
		size_t nScore = _globalPlayers->GetPlayerGlobals(0, isCoop)->GetScore();
		ff::String szText = ff::String::format_new(L"SCORE %02lu", nScore);

		nHighScore = std::max(nHighScore, nScore);

		if (font)
		{
			font->DrawText(render, szText, ff::PointFloat(64, 32), ff::PointFloat(1, 1), ff::PointFloat(0, 0), &s_colorText);
		}
	}
	else if (_globalPlayers->GetPlayerCount() == 2)
	{
		size_t nActive = _globalPlayers->GetCurrentPlayer(0);
		size_t nScore = _globalPlayers->GetPlayerGlobals(0)->GetScore();
		bool bShowName = (nActive != 0 || _counter % 50 < 25);
		ff::String szText = ff::String::format_new(L"%s %02lu", bShowName ? L"1P" : L"  ", nScore);

		nHighScore = std::max(nHighScore, nScore);

		if (font)
		{
			font->DrawText(render, szText, ff::PointFloat(64, 32), ff::PointFloat(1, 1), ff::PointFloat(0, 0), &s_colorText);
		}

		nScore = _globalPlayers->GetPlayerGlobals(1)->GetScore();
		bShowName = (nActive != 1 || _counter % 50 < 25);
		szText = ff::String::format_new(L"%s %02lu", bShowName ? L"2P" : L"  ", nScore);
		nHighScore = std::max(nHighScore, nScore);

		if (font)
		{
			font->DrawText(render, szText, ff::PointFloat(480, 32), ff::PointFloat(1, 1), ff::PointFloat(0, 0), &s_colorText);
		}
	}

	// High score
	if (font)
	{
		ff::String szText = ff::String::format_new(L"HIGH SCORE %02lu", nHighScore);
		font->DrawText(render, szText, ff::PointFloat(1240 /*1312*/, 32), ff::PointFloat(1, 1), ff::PointFloat(0, 0), &s_colorText);
	}

	// Lives
	for (size_t i = 0; i < _globalPlayers->GetPlayerCount(); i++)
	{
		if (i == 0 || !isCoop)
		{
			size_t nLives = isCoop ? _coopLives : _lives[i];

			for (size_t h = 0; h < nLives && h < Globals::GetMaxVisibleLives(); h++)
			{
				ff::PointFloat scale(0.75f, 0.75f);
				DirectX::XMFLOAT4 wheelsColor = isCoop ? ff::GetColorWhite() : Globals::GetPlayerWheelsColor(i);
				DirectX::XMFLOAT4 bodyColor = isCoop ? ff::GetColorWhite() : Globals::GetPlayerBodyColor(i);
				ff::PointFloat pos = Globals::GetPlayerLifePos(i, h);

				wheelsColor.w = 0.5f;
				bodyColor.w = 0.5f;

				render->DrawSprite(_playerWheels, &pos, &scale, 0, &wheelsColor);
				render->DrawSprite(_playerBody, &pos, &scale, 0, &bodyColor);
				render->DrawSprite(_playerTurret, &pos, &scale, 0, &bodyColor);
			}
		}
	}

	// Powerup
	for (size_t i = 0; i < _players->GetPlayerCount(); i++)
	{
		IEntity* pPlayerEntity = _players->GetPlayer(i);
		PlayerComponent* pPlayer = pPlayerEntity->GetComponent<PlayerComponent>();
		float percent = pPlayer->GetPowerupCounterPercent();

		if (percent > 0)
		{
			ff::RectFloat rect = Globals::GetPlayerPowerupArea(pPlayer->GetIndex());

			if (!pPlayer->GetIndex())
			{
				rect.right = rect.left + rect.Width() * percent;
			}
			else
			{
				rect.left = rect.right - rect.Width() * percent;
			}

			DirectX::XMFLOAT4 color = Globals::GetPlayerBodyColor(pPlayer->GetIndex());
			color.w = 0.5f;

			render->DrawFilledRectangle(&rect, &color, 1);
		}
	}
}

void ScoreAdvanceRender::OnEntityEvent(IEntity *entity, ff::hash_t eventName, void *eventArgs)
{
	if (eventName == ENTITY_EVENT_PLAYING_START)
	{
		UpdateLivesCount();
	}
}

void ScoreAdvanceRender::UpdateLivesCount()
{
	bool isCoop = (_globalPlayers->GetGameMode() == GAME_MODE_COOP);

	_coopLives = isCoop ? _globalPlayers->GetCoopGlobals()->GetLives() : 0;

	for (size_t i = 0; i < _globalPlayers->GetPlayerCount(); i++)
	{
		PlayerGlobals* pPlayerGlobals = _globalPlayers->GetPlayerGlobals(i);
		_lives[i] = pPlayerGlobals->GetLives();

		if (isCoop)
		{
			_coopLives += pPlayerGlobals->GetLives();
		}
	}
}
