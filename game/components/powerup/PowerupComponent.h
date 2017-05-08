#pragma once

enum PoweruentityType
{
	POWERUP_TYPE_MULTI_SHOT,
	POWERUP_TYPE_AMMO,
	POWERUP_TYPE_HOMING_SHOT,
	POWERUP_TYPE_SHIELD,
	POWERUP_TYPE_SPEED_BOOST,
	POWERUP_TYPE_BONUS_POINTS,
	POWERUP_TYPE_SPREAD_SHOT,
	POWERUP_TYPE_PUSH_SHOT,

	POWERUP_TYPE_COUNT,
	POWERUP_TYPE_NONE = POWERUP_TYPE_COUNT
};

class __declspec(uuid("254468c6-5457-4ba2-986d-5156c1c8251a"))
	PowerupComponent : public ff::ComBase, public IUnknown
{
public:
	DECLARE_HEADER(PowerupComponent);

	PoweruentityType GetType() const;
	void SetType(PoweruentityType type);

	size_t GetPlayerIndex() const;
	void SetPlayerIndex(size_t index);

private:
	PoweruentityType _type;
	size_t _nPlayerIndex;
};
