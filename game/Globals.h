#pragma once

class InvaderComponent;
class IEntityDomainProvider;
enum BulletEntityType;
enum InvaderEntityType;
enum PoweruentityType;
enum SaucerEntityType;

enum GameMode : int
{
	GAME_MODE_SINGLE,
	GAME_MODE_TURNS,
	GAME_MODE_COOP,

	GAME_MODE_COUNT,
	GAME_MODE_DEFAULT = GAME_MODE_SINGLE
};

enum Difficulty : int
{
	DIFFICULTY_EASY,
	DIFFICULTY_NORMAL,
	DIFFICULTY_HARD,
	DIFFICULTY_BABY,

	DIFFICULTY_COUNT,
	DIFFICULTY_DEFAULT = DIFFICULTY_NORMAL
};

class Globals
{
public:
	static size_t GetAdvancesPerSecond() { return 60; }
	static float GetAdvancesPerSecondF() { return 60.0f; }
	static float GetTransitionTime() { return 1.5f; }

	static size_t GetBonusPoints() { return 1000; }
	static float GetBonusSpeed() { return 1.5f; }
	static float GetBonusDropSpeed() { return 4; }
	static float GetBonusTime(PoweruentityType type, Difficulty diff, size_t nLevel);
	static const DirectX::XMFLOAT4& GetBonusColor(PoweruentityType type);

	static bool HasHighScores(Difficulty diff) { return diff >= DIFFICULTY_EASY && diff <= DIFFICULTY_HARD; }
	static bool IsEasyDifficulty(Difficulty diff) { return diff == DIFFICULTY_EASY || diff == DIFFICULTY_BABY; }
	static bool IsHardDifficulty(Difficulty diff) { return diff == DIFFICULTY_HARD; }
	static size_t GetStartingLives(GameMode mode, Difficulty diff) { return (mode == GAME_MODE_COOP) ? 3 : 2; }
	static size_t GetMaxFreeLives(GameMode mode, Difficulty diff) { return (mode == GAME_MODE_COOP) ? 6 : 3; }
	static size_t GetFirstFreeLife(GameMode mode, Difficulty diff) { return 10000; }
	static size_t GetNextFreeLife(GameMode mode, Difficulty diff) { return (mode == GAME_MODE_COOP) ? 10000 : 20000; }
	static size_t GetMaxLives() { return 6; }
	static size_t GetMaxVisibleLives() { return 6; }
	static size_t GetMaxScore() { return 99999999; }
	static size_t GetMinStartingLives() { return 1; }
	static size_t GetMaxStartingLives() { return 5; }
	static size_t GetMinStartingLevel() { return 0; }
	static size_t GetMaxStartingLevel() { return 15; }

	static ff::PointInt GetLevelGridSize() { return ff::PointInt(50, 50); }
	static ff::PointFloat GetLevelGridSizeF() { return ff::PointFloat(50, 50); }
	static ff::RectInt GetLevelRect() { return ff::RectInt(0, 0, 1900, 1200); }
	static ff::RectFloat GetLevelRectF() { return ff::RectFloat(0, 0, 1900, 1200); }
	static ff::RectFloat GetLevelPlayableRect() { return ff::RectFloat(0, 75, 1900, 1200); }
	static ff::PointInt GetLevelSize() { return ff::PointInt(1900, 1200); }
	static ff::PointFloat GetLevelSizeF() { return ff::PointFloat(1900, 1200); }
	static ff::RectFloat GetLevelGroundRect() { return ff::RectFloat(200, 1050, 1700, 1200); }
	static size_t GetCurrentLevel(IEntityDomainProvider *pDomainProvider);
	static size_t GetInvaderLevel(size_t nLevel) { return nLevel / 2; }
	static Difficulty GetDifficulty(IEntityDomainProvider *pDomainProvider);

	static ff::RectFloat GetSaucerRect(SaucerEntityType type);
	static ff::PointFloat GetSaucerSpeed(SaucerEntityType type, Difficulty diff);
	static size_t GetSaucerMinSeconds() { return 18; }
	static size_t GetSaucerMaxSeconds() { return 22; }
	static size_t GetSaucerBonusPoints(size_t nHitCount) { return nHitCount * 5 + 50; }

	static bool IsBonusLevel(Difficulty diff, size_t nLevel) { return diff != DIFFICULTY_BABY && (nLevel % 2) == 1; }
	static bool IsSaucerBonusLevel(Difficulty diff, size_t nLevel) { return diff != DIFFICULTY_BABY && (nLevel % 4) == 1; }
	static bool IsInvaderBonusLevel(Difficulty diff, size_t nLevel) { return diff != DIFFICULTY_BABY && (nLevel % 4) == 3; }
	static size_t GetSaucerBonusLevel(size_t nLevel) { return nLevel / 4; }
	static size_t GetInvaderBonusLevel(size_t nLevel) { return nLevel / 4; }

	static size_t GetShieldCount() { return 4; }
	static ff::PointFloat GetShieldSize() { return ff::PointFloat(200, 100); }
	static ff::RectFloat GetShieldRect(size_t nShield);

	static ff::RectFloat GetInvaderArea() { return ff::RectFloat(125, 175, 1775, 1025); }
	static ff::PointInt GetInvaderFieldSize() { return ff::PointInt(12, 5); }
	static ff::PointFloat GetInvaderMoveDelta() { return ff::PointFloat(12.5, 25); }
	static ff::PointFloat GetInvaderSpacing() { return ff::PointFloat(100, 100); }
	static size_t GetInvaderPoints(InvaderComponent *pInvader, Difficulty diff);
	static ff::PointFloat GetInvaderBulletOffset() { return ff::PointFloat(0, 20); }
	static float GetInvaderNoShootRow() { return 925; }

	static float GetPlayerMaxSpeed() { return 8.0f; }
	static ff::PointFloat GetPlayerBulletOffset() { return ff::PointFloat(0, -50); }
	static float GetPlayerBulletRepeat() { return 0.5f; }
	static ff::RectFloat GetPlayerMoveArea() { return ff::RectFloat(225, 1050, 1675, 1050); }
	static ff::RectFloat GetPlayerPowerupArea(size_t index);
	static ff::PointFloat GetPlayerLifePos(size_t nPlayerIndex, size_t nLifeIndex);

	static size_t GetBulletPoints(BulletEntityType type);
	static float GetBulletSpeed(BulletEntityType type, Difficulty diff, size_t nInvaderLevel);
	static size_t GetBulletReshootFrames() { return 60; }

	static const DirectX::XMFLOAT4& GetPlayerWheelsColor (size_t index);
	static const DirectX::XMFLOAT4& GetPlayerBodyColor (size_t index);
	static const DirectX::XMFLOAT4& GetPlayerShieldColor (size_t index);

	static float GetJoystickDeadZone() { return 0.4f; }

	static int GetHitTestBoxSize() { return 64; }
	static ff::PointInt GetHitTestBoxCount() { return ff::PointInt(30, 19); } // 1900/64+1, 1200/64+1

	// Strings
	static ff::StaticString IDS_POWERUP_MULTI_SHOT;
	static ff::StaticString IDS_POWERUP_AMMO;
	static ff::StaticString IDS_POWERUP_HOMING_SHOT;
	static ff::StaticString IDS_POWERUP_SHIELD;
	static ff::StaticString IDS_POWERUP_SPEED_BOOST;
	static ff::StaticString IDS_SHOW_SCORE_INTRO;
	static ff::StaticString IDS_SHOW_SCORE_1P;
	static ff::StaticString IDS_SHOW_SCORE_COOP;
	static ff::StaticString IDS_TITLE_FULLSCREENON;
	static ff::StaticString IDS_TITLE_FULLSCREENOFF;
	static ff::StaticString IDS_TITLE_EASY;
	static ff::StaticString IDS_SHOW_SCORE_EASY;
	static ff::StaticString IDS_TITLE_CONTINUE;
	static ff::StaticString IDS_TITLE_EXIT;
	static ff::StaticString IDS_POWERUP_SPREAD_SHOT;
	static ff::StaticString IDS_POWERUP_PUSH_SHOT;
	static ff::StaticString IDS_TITLE_MORE;
	static ff::StaticString IDS_TITLE_HELP;
	static ff::StaticString IDS_TITLE_QUIT;
	static ff::StaticString IDS_TITLE_1P_METRO;
	static ff::StaticString IDS_TITLE_2P_METRO;
	static ff::StaticString IDS_TITLE_COOP_METRO;
	static ff::StaticString IDS_TITLE_EASY_METRO;
	static ff::StaticString IDS_TITLE_NORMAL_METRO;
	static ff::StaticString IDS_TITLE_HARD_METRO;
	static ff::StaticString IDS_TITLE_REPLAY;
	static ff::StaticString IDS_TITLE_HOME;
	static ff::StaticString IDS_TITLE_BABY_METRO;
	static ff::StaticString IDS_SHOW_SCORE_HARD;
	static ff::StaticString IDS_SHOW_SCORE_NORMAL;
	static ff::StaticString IDS_ADD_SCORE_INTRO;
	static ff::StaticString IDS_ADD_SCORE_INTRO2;
	static ff::StaticString IDS_TITLE_PLAY;
	static ff::StaticString IDS_TITLE_OPTIONS;
	static ff::StaticString IDS_TITLE_SCORES;
	static ff::StaticString IDS_TITLE_1P;
	static ff::StaticString IDS_TITLE_2P;
	static ff::StaticString IDS_TITLE_COOP;
	static ff::StaticString IDS_TITLE_FXON;
	static ff::StaticString IDS_TITLE_FXOFF;
	static ff::StaticString IDS_TITLE_NORMAL;
	static ff::StaticString IDS_TITLE_HARD;
	static ff::StaticString IDS_TITLE_VSYNCON;
	static ff::StaticString IDS_TITLE_VSYNCOFF;
};
