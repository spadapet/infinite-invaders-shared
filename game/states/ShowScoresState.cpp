#include "pch.h"
#include "Audio\AudioEffect.h"
#include "COM\ComAlloc.h"
#include "components\core\LoadingComponent.h"
#include "components\graph\SpriteAnimationAdvance.h"
#include "components\graph\SpriteAnimationRender.h"
#include "coreEntity\component\ComponentManager.h"
#include "coreEntity\component\standard\LayerGroupComponent.h"
#include "coreEntity\domain\EntityDomain.h"
#include "coreEntity\entity\Entity.h"
#include "coreEntity\entity\EntityManager.h"
#include "coreEntity\system\SystemManager.h"
#include "Globals.h"
#include "Globals\MetroGlobals.h"
#include "Graph\2D\2dRenderer.h"
#include "Graph\2D\SpriteAnimation.h"
#include "Graph\Anim\AnimKeys.h"
#include "Graph\Anim\AnimPos.h"
#include "Graph\Font\SpriteFont.h"
#include "Graph\RenderTarget\RenderTarget.h"
#include "Input\InputMapping.h"
#include "Input\PointerDevice.h"
#include "InputEvents.h"
#include "Module\Module.h"
#include "Module\ModuleFactory.h"
#include "states\ShowScoresState.h"
#include "systems\AdvanceSystem.h"
#include "systems\RenderSystem.h"
#include "ThisApplication.h"

static ff::ModuleStartup RegisterModuleClass([](ff::Module &module)
{
	ff::StaticString name(L"ShowScoresState");
	module.RegisterClassT<ShowScoresState>(name, __uuidof(IShowScoresState));
});

BEGIN_INTERFACES(ShowScoresState)
	HAS_INTERFACE(IShowScoresState)
	HAS_INTERFACE(ISystem)
END_INTERFACES()

ShowScoresState::ShowScoresState()
	: _underSystemId(GUID_NULL)
	, _gameMode(GAME_MODE_SINGLE)
	, _difficulty(DIFFICULTY_NORMAL)
{
	_szIntro = ff::GetThisModule().GetString(Globals::IDS_SHOW_SCORE_INTRO);
	_sz1P = ff::GetThisModule().GetString(Globals::IDS_SHOW_SCORE_1P);
	_szCOOP = ff::GetThisModule().GetString(Globals::IDS_SHOW_SCORE_COOP);
	_szHard = ff::GetThisModule().GetString(Globals::IDS_SHOW_SCORE_HARD);
	_szNormal = ff::GetThisModule().GetString(Globals::IDS_SHOW_SCORE_NORMAL);
	_szEasy = ff::GetThisModule().GetString(Globals::IDS_SHOW_SCORE_EASY);
}

ShowScoresState::~ShowScoresState()
{
}

// static
bool ShowScoresState::Create(IEntityDomain *pDomain, REFGUID underSystemId, ISystem *pUnderSystem, ISystem **ppState)
{
	assertRetVal(ppState, false);

	ff::ComPtr<ShowScoresState, IShowScoresState> pState;
	assertRetVal(SUCCEEDED(ff::ComAllocator<ShowScoresState>::CreateInstance(
		pDomain, GUID_NULL, __uuidof(ShowScoresState), (void**)&pState)), false);

	assertRetVal(pState->Init(pDomain, underSystemId, pUnderSystem), false);

	*ppState = ff::GetAddRef(pState.Interface());
	return true;
}

bool ShowScoresState::Init(IEntityDomain *pDomain, REFGUID underSystemId, ISystem *pUnderSystem)
{
	ThisApplication *pApp = ThisApplication::Get(pDomain);

	_pUnderSystem = pUnderSystem;
	_underSystemId = underSystemId;
	_gameMode = pApp->GetOptions().GetEnum<GameMode>(ThisApplication::OPTION_GAME_MODE, GAME_MODE_DEFAULT);
	_difficulty = pApp->GetOptions().GetEnum<Difficulty>(ThisApplication::OPTION_DIFFICULTY, DIFFICULTY_DEFAULT);

	// Input mapping
	assertRetVal(CreateInputMapping(true, true, false, &_inputMapping), false);
	assertRetVal(AddDefaultInputEventsAndValues(_inputMapping), false);

	// Load stuff
	_pBackAnim.Init(L"High Score BG");
	_pClassicFont.Init(L"Classic");
	_spaceFont.Init(L"Space");
	_effectMain.Init(L"Menu Main");

	// Child domain
	{
		assertRetVal(CreateEntityDomain(pApp, &_pChildDomain), false);
		assertRetVal(_pChildDomain->GetComponentManager()->CreateComponent<LoadingComponent>(nullptr, &_loading), false);
	}

	// Systems
	{
		_pChildDomain->GetSystemManager()->EnsureSystem<AdvanceSystem>();
		_pChildDomain->GetSystemManager()->EnsureSystem<RenderSystem>();
	}

	// Create global advance/render components
	{
		assertRetVal(_pChildDomain->GetEntityManager()->CreateEntity(&_pBackEntity), false);

		SpriteAnimationRender::Create(_pBackEntity, _pBackAnim.Flush(), ff::POSE_TWEEN_SPLINE_LOOP, LAYER_PRI_LOW);
		SpriteAnimationAdvance *pAnimAdvance = SpriteAnimationAdvance::Create(_pBackEntity);
		pAnimAdvance->SetLoopingAnim(0, 4, 2);
	}

	// Custom render layer
	{
		assertRetVal(Create2dLayerGroup(_pChildDomain, &_pRenderLayer), false);
		_pRenderLayer->SetWorldSize(Globals::GetLevelSizeF());
		_pRenderLayer->GetViewport()->SetAutoSizePadding(pApp->GetOptions().GetRect(ThisApplication::OPTION_WINDOW_PADDING));
		_pRenderLayer->GetViewport()->SetAutoSizeAspect(Globals::GetLevelSize());
		_pRenderLayer->GetViewport()->SetUseDepth(false);
	}

	CacheHighScoreText(pDomain);

	return true;
}

int ShowScoresState::GetSystemPriority() const
{
	return SYS_PRI_STATE_NORMAL;
}

PingResult ShowScoresState::Ping(IEntityDomain *pDomain)
{
	if (_loading)
	{
		_loading->Advance(nullptr);

		if (!_loading->IsLoading())
		{
			_loading = nullptr;
		}
	}

	if (_loading)
	{
		return PING_RESULT_INIT;
	}

	return PING_RESULT_RUNNING;
}

void ShowScoresState::Advance(IEntityDomain *pDomain)
{
	ThisApplication *pApp = ThisApplication::Get(pDomain);
	pApp->AdvanceInputMapping(_inputMapping);

	bool hasEvents = false;
	for (const ff::InputEvent &ie : _inputMapping->GetEvents())
	{
		hasEvents = true;
		if (ie.IsStart())
		{
			HandleEventStart(pDomain, ie._eventID);
		}
		else if (ie.IsStop())
		{
			HandleEventStop(pDomain, ie._eventID);
		}
	}

	ff::IPointerDevice *pointer = ff::MetroGlobals::Get()->GetPointer();
	if (!hasEvents && pointer->GetButtonClickCount(VK_LBUTTON))
	{
		ff::PointFloat mousePos = pointer->GetPos().ToFloat();
		const ff::RectFloat leftRect(425, 275, 525, 350);
		const ff::RectFloat rightRect(875, 275, 975, 350);
		const ff::RectFloat bounds(350, 100, 1425, 1000);

		RenderSystem* render = _pChildDomain->GetSystemManager()->GetSystem<RenderSystem>();
		ff::PointFloat levelPos = render->WindowClientToLevel(mousePos);

		if (leftRect.PointInRect(levelPos))
		{
			HandleEventStart(pDomain, GIE_LEFT);
		}
		else if (rightRect.PointInRect(levelPos))
		{
			HandleEventStart(pDomain, GIE_RIGHT);
		}
		else if (!bounds.PointInRect(levelPos))
		{
			HandleEventStop(pDomain, GIE_BACK);
		}
	}

	_pChildDomain->GetSystemManager()->Advance();
}

void ShowScoresState::Render(IEntityDomain *pDomain, ff::IRenderTarget *pTarget)
{
	if (_pUnderSystem)
	{
		_pUnderSystem->Render(pDomain, pTarget);
	}

	_pChildDomain->GetSystemManager()->Render(pTarget);

	// Render text
	{
		ff::I2dRenderer *render = ff::MetroGlobals::Get()->Get2dRender();
		_pRenderLayer->GetViewport()->SetRenderTargets(&pTarget, 1);

		if (_pRenderLayer->BeginCustomRender(render))
		{
			RenderHighScoreText(pDomain, render);

			_pRenderLayer->EndCustomRender(render);
		}
	}
}

void ShowScoresState::HandleEventStart(IEntityDomain *pDomain, ff::hash_t type)
{
	switch (type)
	{
	case GIE_LEFT:
		AdjustShowScoresIndex(-1);
		CacheHighScoreText(pDomain);
		break;

	case GIE_RIGHT:
		AdjustShowScoresIndex(1);
		CacheHighScoreText(pDomain);
		break;
	}
}

void ShowScoresState::HandleEventStop(IEntityDomain *pDomain, ff::hash_t type)
{
	switch (type)
	{
	case GIE_ACTION:
	case GIE_BACK:
	case GIE_QUIT:
		assertRet(pDomain->GetSystemManager()->HasSystem(this));
		pDomain->GetSystemManager()->RemoveSystem(this);

		if (_pUnderSystem)
		{
			pDomain->GetSystemManager()->AddSystem(_underSystemId, _pUnderSystem);
		}
		break;
	}
}

void ShowScoresState::RenderHighScoreText(IEntityDomain *pDomain, ff::I2dRenderer *render)
{
	bool isCoop = _gameMode == GAME_MODE_COOP;

	ff::ISpriteFont *spaceFont = _spaceFont.GetObject();
	ff::ISpriteFont *classicFont = _pClassicFont.GetObject();

	if (spaceFont)
	{
		DirectX::XMFLOAT4 color(0.82f, 0.45f, 0.17f, 1);

		spaceFont->DrawText(
			render,
			_szIntro,
			ff::PointFloat(450, 175),
			ff::PointFloat(.875, .875),
			ff::PointFloat(0, 0),
			&color);

		spaceFont->DrawText(render,
			isCoop ? _szCOOP : _sz1P,
			ff::PointFloat(525, 292), ff::PointFloat(.5, .5), ff::PointFloat(0, 0), &ff::GetColorWhite());

		spaceFont->DrawText(render,
			Globals::IsHardDifficulty(_difficulty)
			? _szHard
			: (Globals::IsEasyDifficulty(_difficulty) ? _szEasy : _szNormal),
			ff::PointFloat(680, 292), ff::PointFloat(.5, .5), ff::PointFloat(0, 0), &ff::GetColorWhite());
	}

	if (classicFont)
	{
		DirectX::XMFLOAT4 color(1, 1, 1, .625);
		classicFont->DrawText(render, _szHighest, ff::PointFloat(385, 450), ff::PointFloat(1.5, 1.5), ff::PointFloat(0, 0), &ff::GetColorWhite());
		classicFont->DrawText(render, _szOther, ff::PointFloat(425, 600), ff::PointFloat(1, 1), ff::PointFloat(0, 10), &color);
	}
}

static ff::String FormatHighScore(size_t index, const HighScore &score)
{
	ff::String sz;
	size_t nScore = score._stats.GetScore();

	if (index)
	{
		sz.format(L"%2lu.%9lu ", index + 1, nScore);
	}
	else
	{
		sz.format(L"%2lu. %lu ", index + 1, nScore);
	}

	if (score._name[0])
	{
		sz += score._name;
	}
	else
	{
		sz += L"---";
	}

	return sz;
}

void ShowScoresState::CacheHighScoreText(IEntityDomain *pDomain)
{
	ThisApplication* pApp = ThisApplication::Get(pDomain);
	bool isCoop = (_gameMode == GAME_MODE_COOP);
	const HighScores& hs = pApp->GetHighScores();
	size_t nCount = hs.GetHighScoreCount(isCoop, _difficulty);
	HighScore emptyScore;

	ff::ZeroObject(emptyScore);
	_szOther.clear();

	const HighScore &score = nCount ? hs.GetHighScore(isCoop, _difficulty, 0) : emptyScore;
	_szHighest = FormatHighScore(0, score);

	for (size_t i = 1; i < 10; i++)
	{
		const HighScore &score = (i < nCount) ? hs.GetHighScore(isCoop, _difficulty, i) : emptyScore;
		_szOther += FormatHighScore(i, score);
		_szOther += L"\n";
	}
}

static const struct SShowScores
{
	GameMode _gameMode;
	Difficulty _difficulty;
}
s_showScores[] =
{
	{ GAME_MODE_SINGLE, DIFFICULTY_EASY },
	{ GAME_MODE_COOP, DIFFICULTY_EASY },
	{ GAME_MODE_SINGLE, DIFFICULTY_NORMAL },
	{ GAME_MODE_COOP, DIFFICULTY_NORMAL },
	{ GAME_MODE_SINGLE, DIFFICULTY_HARD },
	{ GAME_MODE_COOP, DIFFICULTY_HARD },
};

size_t ShowScoresState::GetShowScoresIndex() const
{
	for (size_t i = 0; i < _countof(s_showScores); i++)
	{
		if (s_showScores[i]._gameMode == _gameMode &&
			s_showScores[i]._difficulty == _difficulty)
		{
			return i;
		}
	}

	assertRetVal(false, 0);
}

size_t ShowScoresState::AdjustShowScoresIndex(int dir)
{
	size_t i = GetShowScoresIndex();

	if (dir < 0)
	{
		i = !i ? _countof(s_showScores) - 1 : i - 1;
	}
	else if (dir > 0)
	{
		i = i + 1 < _countof(s_showScores) ? i + 1 : 0;
	}

	_gameMode = s_showScores[i]._gameMode;
	_difficulty = s_showScores[i]._difficulty;

	ThisApplication* pApp = ThisApplication::Get(_pChildDomain);
	bool bSoundOn = pApp->GetOptions().GetBool(ThisApplication::OPTION_SOUND_ON, true);

	if (bSoundOn && _effectMain.GetObject())
	{
		_effectMain.GetObject()->Play();
	}

	return i;
}
