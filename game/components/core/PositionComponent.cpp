#include "pch.h"
#include "coreEntity\entity\EntityListener.h"
#include "ThisApplication.h"
#include "components\core\PositionComponent.h"
#include "coreEntity\entity\Entity.h"
#include "coreEntity\entity\EntityEvents.h"
#include "entities\EntityEvents.h"
#include "Graph\Anim\AnimPos.h"
#include "Module\ModuleFactory.h"

static PositionInfo s_positionInfoIdentity =
{
	ff::PointFloat(0, 0), // translate
	ff::PointFloat(1, 1), // scale
	0.0f, // rotate
	ff::PointFloat(0, 0), // velocity
	ff::RectFloat(0, 0, 0, 0), // bounds
	{ ff::PointFloat(0, 0), ff::PointFloat(0, 0), ff::PointFloat(0, 0), ff::PointFloat(0, 0) }, // points
};

// static
const PositionInfo &PositionInfo::Identity()
{
	return s_positionInfoIdentity;
}

class __declspec(uuid("fc59aa2a-b602-47b7-8331-d2f934e82892"))
	PositionComponent
		: public ff::ComBase
		, public IPositionComponent
		, public IEntityEventListener
{
public:
	DECLARE_HEADER(PositionComponent);

	void Init(ff::PointFloat pos, ff::PointFloat velocity, const ff::RectFloat *pPosToBox, bool bCollide);

	// IPositionComponent
	virtual bool CanCollide() const override;
	virtual bool HitTest(ff::PointFloat pt) const override;
	virtual bool HitTest(IPositionComponent *pOtherPos) const override;

	virtual ff::PointFloat GetPos() const override;
	virtual ff::PointFloat GetPos(float bankScale) const override;
	virtual void SetPos(ff::PointFloat pos) override;

	virtual ff::PointFloat GetScale() const override;
	virtual void SetScale(ff::PointFloat scale) override;

	virtual float GetRotate() const override;
	virtual void SetRotate(float rotate) override;
	virtual void SetRotateFromVelocity() override;

	virtual ff::PointFloat GetVelocity() const override;
	virtual void SetVelocity(ff::PointFloat velocity) override;
	virtual float GetVelocityAngle() const override;
	virtual float GetVelocityMagnitude() const override;
	virtual float GetVelocityMagnitude2() const override;
	virtual ff::PointFloat GetVelocityNormalized() const override;

	virtual const ff::RectFloat& GetBounds() const override;
	virtual const ff::PointFloat* GetPoints() const override;
	virtual void SetPosToBox(const ff::RectFloat &rect) override;
	virtual const PosToBoxInfo& GetPosToBox() const override;
	virtual const PositionInfo& GetPosInfo() const override;

	virtual const DirectX::XMFLOAT3X3& GetTransform() const override;
	virtual const DirectX::XMFLOAT3X3& GetInverseTransform() const override;

	// IEntityEventListener
	virtual void OnEntityEvent(IEntity *entity, ff::hash_t eventName, void *eventArgs) override;

private:
	void InternalSetRotate(float rotate);
	void InternalSetBoxPoints(ff::RectFloat rect);
	void UpdatePoints();
	void UpdateTransform();
	void SendPositionChangeEvent(const PositionInfo &oldInfo);

	IEntity* _entity;

	PositionInfo _pos;
	PosToBoxInfo _box;

	DirectX::XMFLOAT3X3 _transform; // bounding box to world pos
	DirectX::XMFLOAT3X3 _inverseTransform; // world to bounding box pos

	// flags
	mutable bool _inverseTransformValid;
	bool _canCollide;
	bool _rotateFromVelocity;
};

static ff::ModuleStartup RegisterModuleClass([](ff::Module &module)
{
	ff::StaticString name(L"PositionComponent");
	module.RegisterClassT<PositionComponent>(name, __uuidof(IPositionComponent));
});;

BEGIN_INTERFACES(PositionComponent)
	HAS_INTERFACE(IPositionComponent)
	HAS_INTERFACE(IEntityEventListener)
END_INTERFACES()

PositionComponent::PositionComponent()
	: _entity(nullptr)
	, _inverseTransformValid(true)
	, _canCollide(false)
	, _rotateFromVelocity(false)
{
	ff::CopyObject(_pos, s_positionInfoIdentity);
	ff::ZeroObject(_box);

	_transform = ff::GetIdentityMatrix3x3();
	_inverseTransform = ff::GetIdentityMatrix3x3();
}

PositionComponent::~PositionComponent()
{
	assert(!_entity);
}

// static
IPositionComponent *IPositionComponent::Create(IEntity *entity, ff::PointFloat pos, ff::PointFloat velocity, const ff::RectFloat *pPosToBox, bool bCollide)
{
	assertRetVal(entity, nullptr);

	ff::ComPtr<PositionComponent, IPositionComponent> pPos;
	assertRetVal(entity->CreateComponent<PositionComponent>(&pPos), false);

	pPos->Init(pos, velocity, pPosToBox, bCollide);

	entity->AddComponent<IPositionComponent>(pPos);

	return pPos;
}

void PositionComponent::Init(ff::PointFloat pos, ff::PointFloat velocity, const ff::RectFloat *pPosToBox, bool bCollide)
{
	_pos._translate = pos;
	_transform._31 = pos.x;
	_transform._32 = pos.y;
	_inverseTransformValid = false;

	_pos._velocity = velocity;
	_canCollide = bCollide;

	if (pPosToBox)
	{
		InternalSetBoxPoints(*pPosToBox);
	}

	UpdatePoints();
}

bool PositionComponent::CanCollide() const
{
	return _canCollide;
}

static DirectX::XMMATRIX Get2dTransformMatrix(const DirectX::XMFLOAT3X3 &transform)
{
	DirectX::XMFLOAT4X4A transform4(
		transform._11, transform._12, 0, 0,
		transform._21, transform._22, 0, 0,
		0, 0, 1, 0,
		transform._31, transform._32, 0, 1);

	return DirectX::XMLoadFloat4x4A(&transform4);
}

bool PositionComponent::HitTest(ff::PointFloat pt) const
{
	// transform the point into my bounding box space
	DirectX::XMMATRIX transform = Get2dTransformMatrix(GetInverseTransform());
	DirectX::XMVECTOR ptVector = DirectX::XMVector2Transform(DirectX::XMLoadFloat2((DirectX::XMFLOAT2*)&pt), transform);
	DirectX::XMVECTOR ptCenter = DirectX::XMVectorSet(_box._center.x, _box._center.y, 0, 1);
	DirectX::XMVECTOR ptHalfWidth = DirectX::XMLoadFloat2((DirectX::XMFLOAT2*)&_box._halfWidth);

	return DirectX::XMVector4InBounds(DirectX::XMVectorSubtract(ptVector, ptCenter), ptHalfWidth) != FALSE;
}

// pMine is an AABB
// pOther is any four points that form a 2D rectangle
static bool HitTestHelper(const ff::PointFloat *pMine, const DirectX::XMFLOAT4A *pOther)
{
	DirectX::XMVECTOR otherX = DirectX::XMVectorSet(pOther[0].x, pOther[1].x, pOther[2].x, pOther[3].x);
	DirectX::XMVECTOR otherY = DirectX::XMVectorSet(pOther[0].y, pOther[1].y, pOther[2].y, pOther[3].y);

	if (DirectX::XMVector4LessOrEqual(otherX, DirectX::XMVectorReplicate(pMine[0].x)) ||
		DirectX::XMVector4GreaterOrEqual(otherX, DirectX::XMVectorReplicate(pMine[2].x)) ||
		DirectX::XMVector4LessOrEqual(otherY, DirectX::XMVectorReplicate(pMine[0].y)) ||
		DirectX::XMVector4GreaterOrEqual(otherY, DirectX::XMVectorReplicate(pMine[2].y)))
	{
		return false;
	}

	return true;
}

bool PositionComponent::HitTest(IPositionComponent *pOtherPos) const
{
	assertRetVal(pOtherPos, false);

	const PositionInfo& otherInfo = pOtherPos->GetPosInfo();
	const ff::PointFloat* pOtherPoints = otherInfo._points;

	if ((_pos._points[0].x == _pos._points[1].x || _pos._points[0].y == _pos._points[1].y) &&
		(pOtherPoints[0].x == pOtherPoints[1].x || pOtherPoints[0].y == pOtherPoints[1].y))
	{
		// AABB is good enough when there is no rotation

		return _pos._bounds.DoesTouch(otherInfo._bounds);
	}
	else
	{
		// test two rotated rectangles
		DirectX::XMFLOAT4A points[4];

		// transform the other rectangle into my own coordinate system
		XMVector2TransformStream(points, sizeof(DirectX::XMFLOAT4A),
			(DirectX::XMFLOAT2*)pOtherPoints, sizeof(DirectX::XMFLOAT2), 4,
			Get2dTransformMatrix(GetInverseTransform()));

		if (HitTestHelper(_box._points, points))
		{
			return true;
		}

		// transform my rectangle into the other coordinate system
		XMVector2TransformStream(points, sizeof(DirectX::XMFLOAT4A),
			(DirectX::XMFLOAT2*)GetPoints(), sizeof(DirectX::XMFLOAT2), 4,
			Get2dTransformMatrix(pOtherPos->GetInverseTransform()));

		if (HitTestHelper(pOtherPos->GetPosToBox()._points, points))
		{
			return true;
		}
	}

	return false;
}

ff::PointFloat PositionComponent::GetPos() const
{
	return _pos._translate;
}

ff::PointFloat PositionComponent::GetPos(float bankScale) const
{
	return _pos._translate + _pos._velocity * bankScale;
}

void PositionComponent::SetPos(ff::PointFloat pos)
{
	if (pos != _pos._translate)
	{
		PositionInfo oldInfo = _pos;

		_pos._translate = pos;

		// Update the transform matrix in a sneaky way
		{
			_transform._31 = pos.x;
			_transform._32 = pos.y;
			_inverseTransformValid = false;
		}

		UpdatePoints();
		SendPositionChangeEvent(oldInfo);
	}
}

ff::PointFloat PositionComponent::GetScale() const
{
	return _pos._scale;
}

void PositionComponent::SetScale(ff::PointFloat scale)
{
	if (scale != _pos._scale)
	{
		PositionInfo oldInfo = _pos;

		_pos._scale = scale;

		UpdateTransform();
		UpdatePoints();
		SendPositionChangeEvent(oldInfo);
	}
}

float PositionComponent::GetRotate() const
{
	return _pos._rotate;
}

void PositionComponent::SetRotate(float rotate)
{
	_rotateFromVelocity = false;

	InternalSetRotate(rotate);
}

void PositionComponent::SetRotateFromVelocity()
{
	_rotateFromVelocity = true;

	InternalSetRotate(GetVelocityAngle());
}

ff::PointFloat PositionComponent::GetVelocity() const
{
	return _pos._velocity;
}

void PositionComponent::SetVelocity(ff::PointFloat velocity)
{
	if (velocity != _pos._velocity)
	{
		_pos._velocity = velocity;

		if (_rotateFromVelocity)
		{
			InternalSetRotate(GetVelocityAngle());
		}
	}
}

float PositionComponent::GetVelocityAngle() const
{
	return (_pos._velocity.x || _pos._velocity.y)
		? atan2(-_pos._velocity.y, _pos._velocity.x)
		: 0.0f;
}

float PositionComponent::GetVelocityMagnitude() const
{
	return DirectX::XMVectorGetX(DirectX::XMVector2Length(DirectX::XMLoadFloat2((DirectX::XMFLOAT2*)&_pos._velocity)));
}

float PositionComponent::GetVelocityMagnitude2() const
{
	return DirectX::XMVectorGetX(DirectX::XMVector2LengthSq(DirectX::XMLoadFloat2((DirectX::XMFLOAT2*)&_pos._velocity)));
}

ff::PointFloat PositionComponent::GetVelocityNormalized() const
{
	ff::PointFloat nv;
	XMStoreFloat2((DirectX::XMFLOAT2*)&nv, DirectX::XMVector2Normalize(DirectX::XMLoadFloat2((DirectX::XMFLOAT2*)&_pos._velocity)));

	return nv;
}

const ff::RectFloat &PositionComponent::GetBounds() const
{
	return _pos._bounds;
}

const ff::PointFloat *PositionComponent::GetPoints() const
{
	return _pos._points;
}

void PositionComponent::SetPosToBox(const ff::RectFloat &rect)
{
	if (rect.left != _box._points[0].x ||
		rect.top != _box._points[0].y ||
		rect.right != _box._points[2].x ||
		rect.bottom != _box._points[2].y)
	{
		PositionInfo oldInfo = _pos;

		InternalSetBoxPoints(rect);
		UpdatePoints();
		SendPositionChangeEvent(oldInfo);
	}
}

const PosToBoxInfo &PositionComponent::GetPosToBox() const
{
	return _box;
}

const PositionInfo &PositionComponent::GetPosInfo() const
{
	return _pos;
}

const DirectX::XMFLOAT3X3 &PositionComponent::GetTransform() const
{
	return _transform;
}

const DirectX::XMFLOAT3X3 &PositionComponent::GetInverseTransform() const
{
	if (!_inverseTransformValid)
	{
		DirectX::XMVECTOR det;

		XMStoreFloat3x3(const_cast<DirectX::XMFLOAT3X3*>(&_inverseTransform),
			XMMatrixInverse(&det, XMLoadFloat3x3(&_transform)));

		_inverseTransformValid = true;
	}

	return _inverseTransform;
}

void PositionComponent::OnEntityEvent(IEntity *entity, ff::hash_t eventName, void *eventArgs)
{
	if (eventName == ENTITY_EVENT_ADD_COMPONENT)
	{
		assertSz(!_entity, L"Can't add a PositionComponent to more than entity");
		_entity = entity;
	}
	else if (eventName == ENTITY_EVENT_REMOVE_COMPONENT)
	{
		_entity = nullptr;
	}
}

void PositionComponent::InternalSetRotate(float rotate)
{
	if (rotate != _pos._rotate)
	{
		PositionInfo oldInfo = _pos;

		_pos._rotate = rotate;

		UpdateTransform();
		SendPositionChangeEvent(oldInfo);
	}
}

void PositionComponent::InternalSetBoxPoints(ff::RectFloat rect)
{
	rect.Normalize();

	_box._points[0].SetPoint(rect.left, rect.top);
	_box._points[1].SetPoint(rect.right, rect.top);
	_box._points[2].SetPoint(rect.right, rect.bottom);
	_box._points[3].SetPoint(rect.left, rect.bottom);

	_box._center = rect.Center();
	_box._halfWidth = _box._center - rect.TopLeft();
}

void PositionComponent::UpdatePoints()
{
	DirectX::XMFLOAT4A outPoints[4];
	XMVector2TransformStream(outPoints, sizeof(DirectX::XMFLOAT4A),
		(DirectX::XMFLOAT2*)_box._points, sizeof(DirectX::XMFLOAT2), 4,
		Get2dTransformMatrix(_transform));

	for (size_t i = 0; i < 4; i++)
	{
		_pos._points[i].SetPoint(outPoints[i].x, outPoints[i].y);
	}

	// update axis aligned bounding box (AABB)
	{
		DirectX::XMVECTOR minPoint = XMLoadFloat4A(&outPoints[0]);
		DirectX::XMVECTOR maxPoint = minPoint;

		for (size_t i = 1; i < 4; i++)
		{
			DirectX::XMVECTOR cur = XMLoadFloat4A(&outPoints[i]);
			minPoint = DirectX::XMVectorMin(minPoint, cur);
			maxPoint = DirectX::XMVectorMax(maxPoint, cur);
		}

		XMStoreFloat2((DirectX::XMFLOAT2*)&_pos._bounds.left, minPoint);
		XMStoreFloat2((DirectX::XMFLOAT2*)&_pos._bounds.right, maxPoint);
	}
}

void PositionComponent::UpdateTransform()
{
	XMStoreFloat3x3(&_transform,
		DirectX::XMMatrixAffineTransformation2D(
			DirectX::XMLoadFloat2((DirectX::XMFLOAT2*)&_pos._scale),
			DirectX::XMVectorZero(),
			-_pos._rotate,
			DirectX::XMLoadFloat2((DirectX::XMFLOAT2*)&_pos._points)));

	_inverseTransformValid = false;
}

void PositionComponent::SendPositionChangeEvent(const PositionInfo &oldInfo)
{
	if (_canCollide && _entity)
	{
		PositionChangedEventArgs eventArgs;

		eventArgs._component = this;
		eventArgs._pOldInfo = &oldInfo;
		eventArgs._pNewInfo = &_pos;

		_entity->TriggerEvent(ENTITY_EVENT_POSITION_CHANGED, &eventArgs);
	}
}
