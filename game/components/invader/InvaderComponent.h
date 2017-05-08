#pragma once

enum InvaderEntityType
{
	INVADER_TYPE_0,
	INVADER_TYPE_1,
	INVADER_TYPE_2,
	INVADER_TYPE_3,
	INVADER_TYPE_4,
	INVADER_TYPE_5,
	INVADER_TYPE_6,
	INVADER_TYPE_7,

	INVADER_TYPE_NORMAL_COUNT,

	INVADER_TYPE_BONUS_0 = INVADER_TYPE_NORMAL_COUNT,
	INVADER_TYPE_BONUS_1,
	INVADER_TYPE_BONUS_2,

	INVADER_TYPE_COUNT,
	INVADER_TYPE_DEFAULT = INVADER_TYPE_0
};

enum InvaderMoveType
{
	INVADER_MOVE_NONE,
	INVADER_MOVE_TOGETHER,
	INVADER_MOVE_PATH,
};

class __declspec(uuid("b200e54c-303e-4b86-867b-fb3bc7975864"))
	InvaderComponent : public ff::ComBase, public IUnknown
{
public:
	DECLARE_HEADER(InvaderComponent);

	InvaderEntityType GetType() const;
	void SetType(InvaderEntityType type);

	size_t GetRepeat() const;
	void SetRepeat(size_t nRepeat);

	ff::PointInt GetCell() const;
	void SetCell(ff::PointInt cell);

	InvaderMoveType GetMoveType() const;
	void SetMoveType(InvaderMoveType type);

	bool CanShoot() const;

	bool HasCustomHitBox() const;

private:
	InvaderEntityType _type;
	InvaderMoveType _moveType;
	size_t _repeat;
	ff::PointInt _cell;
};
