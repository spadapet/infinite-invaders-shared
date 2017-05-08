#include "pch.h"
#include "Globals.h"
#include "Graph\Anim\AnimPos.h"
#include "Scores.h"

#include "components\bullet\BulletComponent.h"
#include "components\invader\InvaderComponent.h"
#include "components\powerup\PowerupComponent.h"
#include "components\saucer\SaucerComponent.h"
#include "coreEntity\domain\EntityDomain.h"
#include "coreEntity\domain\EntityDomainProvider.h"
#include "services\GlobalPlayerService.h"

ff::StaticString Globals::IDS_POWERUP_MULTI_SHOT(L"IDS_POWERUP_MULTI_SHOT");
ff::StaticString Globals::IDS_POWERUP_AMMO(L"IDS_POWERUP_AMMO");
ff::StaticString Globals::IDS_POWERUP_HOMING_SHOT(L"IDS_POWERUP_HOMING_SHOT");
ff::StaticString Globals::IDS_POWERUP_SHIELD(L"IDS_POWERUP_SHIELD");
ff::StaticString Globals::IDS_POWERUP_SPEED_BOOST(L"IDS_POWERUP_SPEED_BOOST");
ff::StaticString Globals::IDS_SHOW_SCORE_INTRO(L"IDS_SHOW_SCORE_INTRO");
ff::StaticString Globals::IDS_SHOW_SCORE_1P(L"IDS_SHOW_SCORE_1P");
ff::StaticString Globals::IDS_SHOW_SCORE_COOP(L"IDS_SHOW_SCORE_COOP");
ff::StaticString Globals::IDS_TITLE_FULLSCREENON(L"IDS_TITLE_FULLSCREENON");
ff::StaticString Globals::IDS_TITLE_FULLSCREENOFF(L"IDS_TITLE_FULLSCREENOFF");
ff::StaticString Globals::IDS_TITLE_EASY(L"IDS_TITLE_EASY");
ff::StaticString Globals::IDS_SHOW_SCORE_EASY(L"IDS_SHOW_SCORE_EASY");
ff::StaticString Globals::IDS_TITLE_CONTINUE(L"IDS_TITLE_CONTINUE");
ff::StaticString Globals::IDS_TITLE_EXIT(L"IDS_TITLE_EXIT");
ff::StaticString Globals::IDS_POWERUP_SPREAD_SHOT(L"IDS_POWERUP_SPREAD_SHOT");
ff::StaticString Globals::IDS_POWERUP_PUSH_SHOT(L"IDS_POWERUP_PUSH_SHOT");
ff::StaticString Globals::IDS_TITLE_MORE(L"IDS_TITLE_MORE");
ff::StaticString Globals::IDS_TITLE_HELP(L"IDS_TITLE_HELP");
ff::StaticString Globals::IDS_TITLE_QUIT(L"IDS_TITLE_QUIT");
ff::StaticString Globals::IDS_TITLE_1P_METRO(L"IDS_TITLE_1P_METRO");
ff::StaticString Globals::IDS_TITLE_2P_METRO(L"IDS_TITLE_2P_METRO");
ff::StaticString Globals::IDS_TITLE_COOP_METRO(L"IDS_TITLE_COOP_METRO");
ff::StaticString Globals::IDS_TITLE_EASY_METRO(L"IDS_TITLE_EASY_METRO");
ff::StaticString Globals::IDS_TITLE_NORMAL_METRO(L"IDS_TITLE_NORMAL_METRO");
ff::StaticString Globals::IDS_TITLE_HARD_METRO(L"IDS_TITLE_HARD_METRO");
ff::StaticString Globals::IDS_TITLE_REPLAY(L"IDS_TITLE_REPLAY");
ff::StaticString Globals::IDS_TITLE_HOME(L"IDS_TITLE_HOME");
ff::StaticString Globals::IDS_TITLE_BABY_METRO(L"IDS_TITLE_BABY_METRO");
ff::StaticString Globals::IDS_SHOW_SCORE_HARD(L"IDS_SHOW_SCORE_HARD");
ff::StaticString Globals::IDS_SHOW_SCORE_NORMAL(L"IDS_SHOW_SCORE_NORMAL");
ff::StaticString Globals::IDS_ADD_SCORE_INTRO(L"IDS_ADD_SCORE_INTRO");
ff::StaticString Globals::IDS_ADD_SCORE_INTRO2(L"IDS_ADD_SCORE_INTRO2");
ff::StaticString Globals::IDS_TITLE_PLAY(L"IDS_TITLE_PLAY");
ff::StaticString Globals::IDS_TITLE_OPTIONS(L"IDS_TITLE_OPTIONS");
ff::StaticString Globals::IDS_TITLE_SCORES(L"IDS_TITLE_SCORES");
ff::StaticString Globals::IDS_TITLE_1P(L"IDS_TITLE_1P");
ff::StaticString Globals::IDS_TITLE_2P(L"IDS_TITLE_2P");
ff::StaticString Globals::IDS_TITLE_COOP(L"IDS_TITLE_COOP");
ff::StaticString Globals::IDS_TITLE_FXON(L"IDS_TITLE_FXON");
ff::StaticString Globals::IDS_TITLE_FXOFF(L"IDS_TITLE_FXOFF");
ff::StaticString Globals::IDS_TITLE_NORMAL(L"IDS_TITLE_NORMAL");
ff::StaticString Globals::IDS_TITLE_HARD(L"IDS_TITLE_HARD");
ff::StaticString Globals::IDS_TITLE_VSYNCON(L"IDS_TITLE_VSYNCON");
ff::StaticString Globals::IDS_TITLE_VSYNCOFF(L"IDS_TITLE_VSYNCOFF");

float Globals::GetBonusTime(PoweruentityType type, Difficulty diff, size_t nLevel)
{
	switch (type)
	{
	default:
	case POWERUP_TYPE_MULTI_SHOT:
	case POWERUP_TYPE_HOMING_SHOT:
	case POWERUP_TYPE_SHIELD:
	case POWERUP_TYPE_SPEED_BOOST:
	case POWERUP_TYPE_PUSH_SHOT:
		return Globals::IsEasyDifficulty(diff) ? 12.0f : 10.0f;

	case POWERUP_TYPE_AMMO:
	case POWERUP_TYPE_SPREAD_SHOT:
		return Globals::IsEasyDifficulty(diff) ? 10.0f : 5.0f;

	case POWERUP_TYPE_BONUS_POINTS:
		return 0;
	}
}

static DirectX::XMFLOAT4 MakeFloatColor(BYTE r, BYTE g, BYTE b, BYTE a = 255)
{
	return DirectX::XMFLOAT4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}

const DirectX::XMFLOAT4 &Globals::GetBonusColor(PoweruentityType type)
{
	static DirectX::XMFLOAT4 s_colors[POWERUP_TYPE_COUNT] =
	{
		MakeFloatColor(54, 49, 233), // blue // POWERUP_TYPE_MULTI_SHOT
		MakeFloatColor(240, 74, 93), // red // POWERUP_TYPE_AMMO
		MakeFloatColor(82, 220, 107), // green // POWERUP_TYPE_HOMING_SHOT
		MakeFloatColor(64, 182, 184), // cyan // POWERUP_TYPE_SHIELD
		MakeFloatColor(199, 129, 241), // purple // POWERUP_TYPE_SPEED_BOOST
		MakeFloatColor(196, 181, 12), // yellow // POWERUP_TYPE_BONUS_POINTS
		MakeFloatColor(197, 134, 53), // orange // POWERUP_TYPE_SPREAD_SHOT
		MakeFloatColor(241, 129, 240), // pink // POWERUP_TYPE_PUSH_SHOT
	};

	if (type >= 0 && type < _countof(s_colors))
	{
		return s_colors[type];
	}

	return ff::GetColorWhite();
}

ff::RectFloat Globals::GetShieldRect(size_t nShield)
{
	switch (nShield)
	{
	default:
		assert(false);
		__fallthrough;

	case 0:
		return ff::RectFloat(250, 800, 450, 900);

	case 1:
		return ff::RectFloat(600, 800, 800, 900);

	case 2:
		return ff::RectFloat(1100, 800, 1300, 900);

	case 3:
		return ff::RectFloat(1450, 800, 1650, 900);
	}
}

size_t Globals::GetInvaderPoints(InvaderComponent *pInvader, Difficulty diff)
{
	switch (pInvader->GetType())
	{
	default:
		{
			size_t nInvader = pInvader->GetRepeat() ? INVADER_TYPE_COUNT : (pInvader->GetType() - INVADER_TYPE_0);
			size_t nPoints = Globals::IsHardDifficulty(diff) ? 20 : 10;
			nPoints += nInvader * 10;
			return nPoints;
		}

	case INVADER_TYPE_BONUS_0:
		return 20;

	case INVADER_TYPE_BONUS_1:
		return 30;

	case INVADER_TYPE_BONUS_2:
		return 40;
	}
}

ff::RectFloat Globals::GetPlayerPowerupArea(size_t index)
{
	return (index == 0)
		? ff::RectFloat(250, 1160, 900, 1185)
		: ff::RectFloat(1000, 1160, 1650, 1185);
}

ff::PointFloat Globals::GetPlayerLifePos(size_t nPlayerIndex, size_t nLifeIndex)
{
	ff::PointFloat pos = Globals::GetLevelGroundRect().TopLeft();

	if (!nPlayerIndex)
	{
		pos.Offset(100, 85);
		pos.Offset(100.0f * nLifeIndex, 0);
	}
	else
	{
		pos.Offset(1400, 85);
		pos.Offset(-100.0f * nLifeIndex, 0);
	}

	return pos;
}

static const DirectX::XMFLOAT4 s_wheelsColors[2] =
{
	DirectX::XMFLOAT4(1, 1, 1, 1),
	DirectX::XMFLOAT4(1, 1, 1, 1),
};

static const DirectX::XMFLOAT4 s_bodyColors[2] =
{
	DirectX::XMFLOAT4(1, 1, .85f, 1),
	DirectX::XMFLOAT4(.85f, .85f, 1, 1),
};

static const DirectX::XMFLOAT4 s_playerShieldColors[2] =
{
	DirectX::XMFLOAT4(1, 1, .1f, 1),
	DirectX::XMFLOAT4(.1f, .1f, 1, 1),
};

const DirectX::XMFLOAT4 &Globals::GetPlayerWheelsColor(size_t index)
{
	assertRetVal(index < _countof(s_wheelsColors), s_wheelsColors[0]);
	return s_wheelsColors[index];
}

const DirectX::XMFLOAT4 &Globals::GetPlayerBodyColor(size_t index)
{
	assertRetVal(index < _countof(s_bodyColors), s_bodyColors[0]);
	return s_bodyColors[index];
}

const DirectX::XMFLOAT4 &Globals::GetPlayerShieldColor(size_t index)
{
	assertRetVal(index < _countof(s_playerShieldColors), s_playerShieldColors[0]);
	return s_playerShieldColors[index];
}

ff::RectFloat Globals::GetSaucerRect(SaucerEntityType type)
{
	switch (type)
	{
	case SAUCER_TYPE_BONUS_LEVEL:
	case SAUCER_TYPE_BONUS_LEVEL_FAST:
		return ff::RectFloat(-75, 450, 1975, 500);

	default:
		return ff::RectFloat(-75, 100, 1975, 150);
	}
}

ff::PointFloat Globals::GetSaucerSpeed(SaucerEntityType type, Difficulty diff)
{
	switch (type)
	{
	case SAUCER_TYPE_BONUS_LEVEL:
		return Globals::IsHardDifficulty(diff) ? ff::PointFloat(4, 0) : ff::PointFloat(3, 0);

	case SAUCER_TYPE_BONUS_LEVEL_FAST:
		return Globals::IsHardDifficulty(diff) ? ff::PointFloat(7, 0) : ff::PointFloat(6, 0);

	default:
		switch (diff)
		{
		case DIFFICULTY_HARD:
			return ff::PointFloat(4.5, 0);

		case DIFFICULTY_EASY:
			return ff::PointFloat(3, 0);

		case DIFFICULTY_BABY:
			return ff::PointFloat(2, 0);

		default:
			return ff::PointFloat(4, 0);
		}
		break;
	}
}

size_t Globals::GetCurrentLevel(IEntityDomainProvider *pDomainProvider)
{
	ff::ComPtr<GlobalPlayerService> pPlayers;
	assertRetVal(GetService(pDomainProvider, &pPlayers), 0);
	assertRetVal(pPlayers->GetCurrentPlayerCount(), 0);

	return pPlayers->GetCurrentPlayerGlobals(0, true)->GetLevel();
}

Difficulty Globals::GetDifficulty(IEntityDomainProvider *pDomainProvider)
{
	ff::ComPtr<GlobalPlayerService> pPlayers;
	Difficulty diff = DIFFICULTY_NORMAL;

	if (GetService(pDomainProvider, &pPlayers))
	{
		diff = pPlayers->GetDifficulty();
	}

	return diff;
}

size_t Globals::GetBulletPoints(BulletEntityType type)
{
	switch (type)
	{
	case BULLET_TYPE_INVADER_SMALL:
		return 10;

	case BULLET_TYPE_INVADER_LARGE:
		return 50;

	default:
		return 0;
	}
}

float Globals::GetBulletSpeed(BulletEntityType type, Difficulty diff, size_t nInvaderLevel)
{
	switch (type)
	{
	case BULLET_TYPE_PLAYER_0:
	case BULLET_TYPE_PLAYER_1:
	case BULLET_TYPE_HOMING_PLAYER_0:
	case BULLET_TYPE_HOMING_PLAYER_1:
	case BULLET_TYPE_SPREAD_PLAYER_0:
	case BULLET_TYPE_SPREAD_PLAYER_1:
		return 18;

	case BULLET_TYPE_PUSH_PLAYER_0:
	case BULLET_TYPE_PUSH_PLAYER_1:
		return 16;

	case BULLET_TYPE_FAST_PLAYER_0:
	case BULLET_TYPE_FAST_PLAYER_1:
		return 26;

	case BULLET_TYPE_INVADER_LARGE:
		switch (diff)
		{
		case DIFFICULTY_HARD: return 7.0f;
		default: return 6.0f;
		case DIFFICULTY_EASY: return 5.0f;
		case DIFFICULTY_BABY: return 3.0f;
		}

	default:
		switch (diff)
		{
		case DIFFICULTY_BABY:
			return 3.0f;

		case DIFFICULTY_EASY:
			return 6.0f;

		case DIFFICULTY_HARD:
			return std::min(11.0f, 8.0f + nInvaderLevel / 2);

		default:
			return std::min(10.0f, 6.0f + nInvaderLevel / 3);
		}
	}
}
