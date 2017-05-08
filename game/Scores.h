#pragma once

#include "Globals.h"

class PlayerGlobals
{
public:
	PlayerGlobals();

	size_t GetScore() const;
	void SetScore(size_t nScore);
	void AddScore(size_t nScore);

	size_t GetLives() const;
	void SetLives(size_t nLives);
	void AddLives(size_t nLives);
	void RemoveLives(size_t nLives);
	void SetFreeLifeScore(size_t nFirstLife, size_t nNextLife);

	Difficulty GetDifficulty() const;
	void SetDifficulty(Difficulty diff);

	GameMode GetGameMode() const;
	void SetGameMode(GameMode mode);

	size_t GetLevel() const;
	void SetLevel(size_t nLevel);

	bool IsActive() const;
	void SetActive(bool bActive);

private:
	size_t _nLevel;
	size_t _nScore;
	size_t _nFreeLifeScore;
	size_t _nFirstLife;
	size_t _nIncrementLife;
	size_t _nLives;
	Difficulty _diff;
	GameMode _gameMode;
	bool _bActive;
};

struct HighScore
{
	static const size_t s_nNameCount = 10;

	PlayerGlobals _stats;
	FILETIME _time;
	TCHAR _name[s_nNameCount];
};

class __declspec(uuid("098f08b1-fbf7-4e8d-80b7-0af5668e6358"))
	HighScores
{
public:
	HighScores();
	HighScores(const HighScores &rhs);
	~HighScores();

	HighScores &operator=(const HighScores &rhs);

	void Clear();

	size_t GetHighScoreCount(bool isCoop, Difficulty diff) const;
	const HighScore& GetHighScore(bool isCoop, Difficulty diff, size_t index) const;

	bool IsHighScore(bool isCoop, const PlayerGlobals &stats, size_t *pPos = nullptr) const;
	bool InsertHighScore(bool isCoop, const PlayerGlobals &stats, LPCTSTR name);

private:
	static const size_t s_count = 10;
	static const size_t s_diffCount = DIFFICULTY_HARD + 1;

	HighScore _highScores[s_diffCount][s_count];
	HighScore _coopHighScores[s_diffCount][s_count];
	HighScore _emptyScore;
};
