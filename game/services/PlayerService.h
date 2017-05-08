#pragma once

class IEntity;

class __declspec(uuid("ab111c7f-f5a4-41e9-96bd-7f917af2103a"))
	IPlayerService : public IUnknown
{
public:
	virtual size_t GetPlayerCount() const = 0;
	virtual IEntity* GetPlayer(size_t index) const = 0;

	virtual bool IsPlayerAlive(size_t index) const = 0;
	virtual bool IsAnyPlayerAlive() const = 0;
	virtual bool AreAllPlayersDead() const = 0;
	virtual bool AreAllPlayersOutOfLives() const = 0;
};
