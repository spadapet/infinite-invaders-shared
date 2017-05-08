#include "pch.h"
#include "Scores.h"
#include "App.xaml.h"
#include "Globals\MetroGlobals.h"

PlayerGlobals::PlayerGlobals()
{
	ff::ZeroObject(*this);
}

size_t PlayerGlobals::GetScore() const
{
	return _nScore;
}

void PlayerGlobals::SetScore(size_t nScore)
{
	_nScore = std::min(nScore, Globals::GetMaxScore());
}

void PlayerGlobals::AddScore(size_t nScore)
{
	SetScore(_nScore + nScore);

	if (_nFreeLifeScore &&
		_nIncrementLife &&
		_nScore >= _nFreeLifeScore &&
		(_nFreeLifeScore - _nFirstLife) / _nIncrementLife < Globals::GetMaxFreeLives(_gameMode, _diff))
	{
		_nFreeLifeScore = _nIncrementLife ? (_nIncrementLife + _nFreeLifeScore) : 0;
		AddLives(1);
	}
}

size_t PlayerGlobals::GetLives() const
{
	return _nLives;
}

void PlayerGlobals::SetLives(size_t nLives)
{
	_nLives = std::min(nLives, Globals::GetMaxLives());
}

void PlayerGlobals::AddLives(size_t nLives)
{
	SetLives(_nLives + nLives);
}

void PlayerGlobals::RemoveLives(size_t nLives)
{
	SetLives(nLives >= _nLives ? 0 : _nLives - nLives);
}

void PlayerGlobals::SetFreeLifeScore(size_t nFirstLife, size_t nNextLife)
{
	_nFreeLifeScore = nFirstLife;
	_nFirstLife = nFirstLife;
	_nIncrementLife = nNextLife;
}

Difficulty PlayerGlobals::GetDifficulty() const
{
	return _diff;
}

void PlayerGlobals::SetDifficulty(Difficulty diff)
{
	_diff = diff;
}

GameMode PlayerGlobals::GetGameMode() const
{
	return _gameMode;
}

void PlayerGlobals::SetGameMode(GameMode mode)
{
	_gameMode = mode;
}

size_t PlayerGlobals::GetLevel() const
{
	return _nLevel;
}

void PlayerGlobals::SetLevel(size_t nLevel)
{
	_nLevel = nLevel;
}

bool PlayerGlobals::IsActive() const
{
	return _bActive;
}

void PlayerGlobals::SetActive(bool bActive)
{
	_bActive = bActive;
}

HighScores::HighScores()
{
	ff::ZeroObject(*this);

	size_t nScore = 1000;

	static const LPCTSTR names[] =
	{
		L"FOO",
		L"BAR",
		L"BAZ",
		L"QUX",
		L"GOO",
		L"GAR",
		L"GAZ",
		L"GUX",
		L"FAR",
		L"FRED",
	};

	for (size_t i = ff::PreviousSize(s_count); i != ff::INVALID_SIZE; i = ff::PreviousSize(i))
	{
		for (size_t h = 0; h < s_diffCount; h++)
		{
			_highScores[h][i]._stats.SetScore(nScore);
			_tcscpy_s(_highScores[h][i]._name, names[i % _countof(names)]);

			_coopHighScores[h][i]._stats.SetScore(nScore);
			_tcscpy_s(_coopHighScores[h][i]._name, names[i % _countof(names)]);
		}

		nScore += 500;
	}
}

HighScores::HighScores(const HighScores &rhs)
{
	*this = rhs;
}

HighScores::~HighScores()
{
}

HighScores &HighScores::operator=(const HighScores &rhs)
{
	if (this != &rhs)
	{
		CopyMemory(this, &rhs, sizeof(rhs));
	}

	return *this;
}

void HighScores::Clear()
{
	ff::ZeroObject(*this);
}

size_t HighScores::GetHighScoreCount(bool isCoop, Difficulty diff) const
{
	assertRetVal(diff >= 0 && (size_t)diff < s_diffCount, 0);

	const HighScore *pScores = isCoop
		? _coopHighScores[diff]
		: _highScores[diff];

	for (size_t i = 0; i < s_count; i++)
	{
		if (!pScores[i]._stats.GetScore())
		{
			return i;
		}
	}

	return s_count;
}

const HighScore &HighScores::GetHighScore(bool isCoop, Difficulty diff, size_t index) const
{
	assertRetVal(diff >= 0 && (size_t)diff < s_diffCount, _emptyScore);
	assertRetVal(index >= 0 && index < GetHighScoreCount(isCoop, diff), _emptyScore);

	return isCoop
		? _coopHighScores[diff][index]
		: _highScores[diff][index];
}

bool HighScores::IsHighScore(bool isCoop, const PlayerGlobals &stats, size_t *pPos) const
{
	const HighScore *pScores = isCoop
		? _coopHighScores[stats.GetDifficulty()]
		: _highScores[stats.GetDifficulty()];

	for (size_t i = 0; i < s_count; i++)
	{
		if (stats.GetScore() > pScores[i]._stats.GetScore())
		{
			if (pPos)
			{
				*pPos = i;
			}

			return true;
		}
	}

	return false;
}

bool HighScores::InsertHighScore(bool isCoop, const PlayerGlobals &stats, LPCTSTR name)
{
	name = name ? name : L"";

	HighScore *pScores = isCoop
		? _coopHighScores[stats.GetDifficulty()]
		: _highScores[stats.GetDifficulty()];

	for (size_t i = 0; i < s_count; i++)
	{
		if (stats.GetScore() > pScores[i]._stats.GetScore())
		{
			for (size_t h = ff::PreviousSize(s_count); h != ff::INVALID_SIZE && h > i; h = ff::PreviousSize(h))
			{
				pScores[h] = pScores[h - 1];
			}

			pScores[i]._stats = stats;
			_sntprintf_s(pScores[i]._name, _countof(pScores[i]._name), _TRUNCATE, L"%s", name);

			SYSTEMTIME st;
			GetSystemTime(&st);
			SystemTimeToFileTime(&st, &pScores[i]._time);

			// Save scores immediately in case the app gets killed
			ff::MetroGlobals::Get()->SaveState();

			return true;
		}
	}

	return false;
}
