#pragma once

class IEntity;

struct PositionInfo
{
	static const PositionInfo &Identity();

	ff::PointFloat _translate;
	ff::PointFloat _scale;
	float _rotate;
	ff::PointFloat _velocity;

	ff::RectFloat _bounds; // AABB for _points
	ff::PointFloat _points[4]; // tl, tr, br, bl
};

struct PosToBoxInfo
{
	ff::PointFloat _points[4];
	ff::PointFloat _center;
	ff::PointFloat _halfWidth;
};

class __declspec(uuid("bca47da9-0516-4292-b248-7147b2ae48a5")) __declspec(novtable)
	IPositionComponent : public IUnknown
{
public:
	static IPositionComponent *Create(IEntity *entity, ff::PointFloat pos, ff::PointFloat velocity, const ff::RectFloat *pPosToBox, bool bCollide);

	virtual bool CanCollide() const = 0;
	virtual bool HitTest(ff::PointFloat pt) const = 0;
	virtual bool HitTest(IPositionComponent *pOtherPos) const = 0;

	virtual ff::PointFloat GetPos() const = 0;
	virtual ff::PointFloat GetPos(float bankScale) const = 0;
	virtual void SetPos(ff::PointFloat pos) = 0;

	virtual ff::PointFloat GetScale() const = 0;
	virtual void SetScale(ff::PointFloat scale) = 0;

	virtual float GetRotate() const = 0;
	virtual void SetRotate(float rotate) = 0;
	virtual void SetRotateFromVelocity() = 0;

	virtual ff::PointFloat GetVelocity() const = 0;
	virtual void SetVelocity(ff::PointFloat velocity) = 0;
	virtual float GetVelocityAngle() const = 0;
	virtual float GetVelocityMagnitude() const = 0;
	virtual float GetVelocityMagnitude2() const = 0;
	virtual ff::PointFloat GetVelocityNormalized() const = 0;

	virtual const ff::RectFloat& GetBounds() const = 0;
	virtual const ff::PointFloat* GetPoints() const = 0; // always returns four points (a rotated rectangle)
	virtual void SetPosToBox(const ff::RectFloat &rect) = 0;
	virtual const PosToBoxInfo& GetPosToBox() const = 0;
	virtual const PositionInfo& GetPosInfo() const = 0;

	virtual const DirectX::XMFLOAT3X3& GetTransform() const = 0;
	virtual const DirectX::XMFLOAT3X3& GetInverseTransform() const = 0;
};
