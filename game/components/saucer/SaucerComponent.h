#pragma once

enum SaucerEntityType
{
	SAUCER_TYPE_MULTI_SHOT,
	SAUCER_TYPE_AMMO,
	SAUCER_TYPE_HOMING_SHOT,
	SAUCER_TYPE_SHIELD,
	SAUCER_TYPE_SPEED_BOOST,
	SAUCER_TYPE_POINTS,
	SAUCER_TYPE_SPREAD_SHOT,
	SAUCER_TYPE_PUSH_SHOT,
	SAUCER_TYPE_NONE,

	SAUCER_TYPE_BONUS_LEVEL,
	SAUCER_TYPE_BONUS_LEVEL_FAST,

	SAUCER_TYPE_COUNT
};

class __declspec(uuid("51e523e4-874e-4132-8614-f8bec5b44171"))
	SaucerComponent : public ff::ComBase, public IUnknown
{
public:
	DECLARE_HEADER(SaucerComponent);

	SaucerEntityType GetType() const;
	void SetType(SaucerEntityType type);
	bool IsBonus() const;

	size_t GetHitCount() const;
	void AddHitCount();

private:
	SaucerEntityType _type;
	size_t _hitCount;
};
