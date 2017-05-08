#include "pch.h"
#include "Globals.h"
#include "InputEvents.h"
#include "ThisApplication.h"
#include "components\bullet\BulletComponent.h"
#include "components\core\CollisionAdvanceRender.h"
#include "coreEntity\component\standard\LayerGroupComponent.h"
#include "coreEntity\domain\EntityDomain.h"
#include "coreEntity\domain\EntityDomainProvider.h"
#include "coreEntity\entity\Entity.h"
#include "entities\EntityEvents.h"
#include "Graph\2D\2dRenderer.h"
#include "Input\InputMapping.h"
#include "Module\ModuleFactory.h"

static const int s_nGridSize = 64;
static const float s_fGridSize = 64;

enum CollisionInputEvent
{
	CIE_TOGGLE_GRID,
};

static const ff::InputEventMapping s_collisionInputEvents[] =
{
	{ { { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, 'G' } }, CIE_TOGGLE_GRID },
};

static ff::ModuleStartup RegisterModuleClass([](ff::Module &module)
{
	ff::StaticString name(L"CollisionAdvanceRender");
	module.RegisterClassT<CollisionAdvanceRender>(name);
});

BEGIN_INTERFACES(CollisionAdvanceRender)
	HAS_INTERFACE(IAdvanceComponent)
	HAS_INTERFACE(I2dRenderComponent)
	HAS_INTERFACE(IEntityEventListener)
	HAS_INTERFACE(IComponentListener)
END_INTERFACES()

CollisionAdvanceRender::CollisionAdvanceRender()
	: _renderGrid(false)
{
	_cellToEntity.SetBucketCount(512, false);
	_entityToCells.SetBucketCount(512, false);
}

CollisionAdvanceRender::~CollisionAdvanceRender()
{
}

HRESULT CollisionAdvanceRender::_Construct(IUnknown *unkOuter)
{
	ff::ComPtr<IEntityDomainProvider> pDomainProvider;
	assertRetVal(pDomainProvider.QueryFrom(unkOuter), E_INVALIDARG);
	ThisApplication *pApp = ThisApplication::Get(pDomainProvider);

	_positionListener.Init(pDomainProvider->GetDomain(), this);
	_positionChangedListener.Init(pDomainProvider->GetDomain(), ENTITY_EVENT_POSITION_CHANGED, this);
	_collisionListener.Init(pDomainProvider->GetDomain(), ENTITY_EVENT_COLLISION, this);

#ifdef _DEBUG
	// Init input events
	{
		assertRetVal(CreateInputMapping(true, false, false, &_inputMap), E_FAIL);
		assertRetVal(_inputMap->MapEvents(s_collisionInputEvents, _countof(s_collisionInputEvents)), E_FAIL);
	}
#endif

	return __super::_Construct(unkOuter);
}

int CollisionAdvanceRender::GetAdvancePriority() const
{
	return LAYER_PRI_HIGH;
}

void CollisionAdvanceRender::Advance(IEntity *entity)
{
	AdvanceInput(entity);

	DetectCollisions();

	TriggerCollisionEvents();
}

void CollisionAdvanceRender::AdvanceInput(IEntity *entity)
{
	if (_inputMap)
	{
		ThisApplication *pApp = ThisApplication::Get(entity);
		pApp->AdvanceInputMapping(_inputMap);

		for (const ff::InputEvent &ie : _inputMap->GetEvents())
		{
			if (ie.IsStart())
			{
				switch (ie._eventID)
				{
				case CIE_TOGGLE_GRID:
					_renderGrid = !_renderGrid;
					break;
				}
			}
		}
	}
}

int CollisionAdvanceRender::Get2dRenderPriority() const
{
	return LAYER_PRI_HIGH;
}

void CollisionAdvanceRender::Render(IEntity *entity, I2dLayerGroup *group, ff::I2dRenderer *render)
{
#ifdef _DEBUG
	if (_renderGrid)
	{
		ff::PointFloat size = Globals::GetLevelSizeF();
		DirectX::XMFLOAT4 gridColor(1, 0, 0, 0.1875f);
		DirectX::XMFLOAT4 boxColor(0, 1, 0, 0.50f);
		DirectX::XMFLOAT4 pointsColor(0, 1, 0, 1);

		for (float i = 0; i <= size.x; i += s_fGridSize)
		{
			render->DrawLine(&ff::PointFloat(i, 0), &ff::PointFloat(i, size.y), &gridColor);
		}

		for (float i = 0; i <= size.y; i += s_fGridSize)
		{
			render->DrawLine(&ff::PointFloat(0, i), &ff::PointFloat(size.x, i), &gridColor);
		}

		render->Flush();
		render->NudgeDepth();

		for (ff::BucketIter i = _entityToCells.StartIteration(); i != ff::INVALID_ITER; i = _entityToCells.Iterate(i))
		{
			const EntityPosition& ep = _entityToCells.KeyAt(i);
			const ff::PointFloat* pPoints = ep._component->GetPoints();

			render->DrawRectangle(&ep._component->GetBounds(), &boxColor);

			if (pPoints[0].x != pPoints[3].x)
			{
				render->DrawClosedPolyLine(pPoints, 4, &pointsColor);
			}

			const float offset = 5;
			ff::PointFloat pt = ep._component->GetPos();
			render->DrawLine(&ff::PointFloat(pt.x - offset, pt.y - offset), &ff::PointFloat(pt.x + offset, pt.y + offset), &boxColor);
			render->DrawLine(&ff::PointFloat(pt.x - offset, pt.y + offset), &ff::PointFloat(pt.x + offset, pt.y - offset), &boxColor);
		}
	}
#endif
}

void CollisionAdvanceRender::OnEntityEvent(IEntity *entity, ff::hash_t eventName, void *eventArgs)
{
	if (eventName == ENTITY_EVENT_POSITION_CHANGED)
	{
		const PositionChangedEventArgs &eventArgs2 = *(const PositionChangedEventArgs*)eventArgs;

		ff::RectInt oldCellRect;
		GetCellRect(eventArgs2._pOldInfo->_bounds, oldCellRect);

		ff::RectInt newCellRect;
		GetCellRect(eventArgs2._pNewInfo->_bounds, newCellRect);

		if (oldCellRect != newCellRect)
		{
			EntityPosition entityPos(entity, eventArgs2._component);
			RemoveCells(entityPos);
			AddCells(entityPos, newCellRect);
		}
	}
	else if (eventName == ENTITY_EVENT_COLLISION)
	{
		const CollisionEventArgs &eventArgs2 = *(const CollisionEventArgs*)eventArgs;

		HandleCollision(entity, eventArgs2._pOther);
	}
}

void CollisionAdvanceRender::OnAddComponent(IEntity *entity, REFGUID compId, IUnknown *pComp)
{
	ff::ComPtr<IPositionComponent> pPos;
	assertRet(pPos.QueryFrom(pComp));

	if (pPos->CanCollide()) // this never changes
	{
		ff::RectInt cellRect;
		GetCellRect(pPos->GetBounds(), cellRect);

		AddCells(EntityPosition(entity, pPos), cellRect);
	}
}

void CollisionAdvanceRender::OnRemoveComponent(IEntity *entity, REFGUID compId, IUnknown *pComp)
{
	ff::ComPtr<IPositionComponent> pPos;
	assertRet(pPos.QueryFrom(pComp));

	if (pPos->CanCollide())
	{
		RemoveCells(EntityPosition(entity, pPos));
	}
}

void CollisionAdvanceRender::AddCells(EntityPosition entityPos, const ff::RectInt &cellRect)
{
	assert(!_entityToCells.Exists(entityPos));
	_entityToCells.SetKey(entityPos, cellRect);

	for (int y = cellRect.top; y <= cellRect.bottom; y++)
	{
		for (int x = cellRect.left; x <= cellRect.right; x++)
		{
			ff::PointShort cell(x, y);
			_cellToEntity.Insert(cell, entityPos);
		}
	}
}

void CollisionAdvanceRender::RemoveCells(EntityPosition entityPos)
{
	ff::BucketIter i = _entityToCells.Get(entityPos);

	if (i != ff::INVALID_ITER)
	{
		ff::RectInt cellRect = _entityToCells.ValueAt(i);
		_entityToCells.DeletePos(i);

		for (int y = cellRect.top; y <= cellRect.bottom; y++)
		{
			for (int x = cellRect.left; x <= cellRect.right; x++)
			{
				ff::PointShort cell(x, y);

				for (i = _cellToEntity.Get(cell); i != ff::INVALID_ITER; i = _cellToEntity.GetNext(i))
				{
					if (_cellToEntity.ValueAt(i) == entityPos)
					{
						_cellToEntity.DeletePos(i);
						break;
					}
				}
			}
		}
	}
}

void CollisionAdvanceRender::GetCellRect(const ff::RectFloat &rect, ff::RectInt &cellRect)
{
	DirectX::XMVECTOR vec = DirectX::XMVectorDivide(
		DirectX::XMLoadFloat4((const DirectX::XMFLOAT4*)&rect),
		DirectX::XMVectorReplicate(s_fGridSize));

	DirectX::XMStoreSInt4((DirectX::XMINT4*)&cellRect, vec);
}

void CollisionAdvanceRender::DetectCollisions()
{
	assert(!_collisions.Size());

	for (ff::BucketIter i = _cellToEntity.StartIteration(); i != ff::INVALID_ITER; i = _cellToEntity.Iterate(i))
	{
		const EntityPosition& leftPos = _cellToEntity.ValueAt(i);

		for (ff::BucketIter iNext = _cellToEntity.GetNext(i); iNext != ff::INVALID_ITER; iNext = _cellToEntity.GetNext(iNext))
		{
			const EntityPosition& rightPos = _cellToEntity.ValueAt(iNext);

			if (leftPos._component->HitTest(rightPos._component))
			{
				_collisions.SetKey(std::make_tuple(
					std::min(leftPos._entity, rightPos._entity),
					std::max(leftPos._entity, rightPos._entity)));
			}
		}
	}
}

void CollisionAdvanceRender::TriggerCollisionEvents()
{
	ff::Vector<ff::ComPtr<IEntity>, 32> keepAliveEntities;
	keepAliveEntities.Reserve(_collisions.Size() * 2);

	for (ff::BucketIter i = _collisions.StartIteration(); i != ff::INVALID_ITER; i = _collisions.Iterate(i))
	{
		const CollisionPair &pair = _collisions.KeyAt(i);

		keepAliveEntities.Push(std::get<0>(pair));
		keepAliveEntities.Push(std::get<1>(pair));
	}

	for (ff::BucketIter i = _collisions.StartIteration(); i != ff::INVALID_ITER; i = _collisions.Iterate(i))
	{
		const CollisionPair &pair = _collisions.KeyAt(i);

		CollisionEventArgs args1;
		CollisionEventArgs args2;

		args1._pOther = std::get<1>(pair);
		args2._pOther = std::get<0>(pair);

		std::get<0>(pair)->TriggerEvent(ENTITY_EVENT_COLLISION, &args1);
		std::get<1>(pair)->TriggerEvent(ENTITY_EVENT_COLLISION, &args2);
	}

	_collisions.Clear();
}

void CollisionAdvanceRender::HandleCollision(IEntity *entity, IEntity *pOther)
{
	// don't really need this function, might be useful for debugging collisions though
}

bool operator<(const ff::PointShort &lhs, const ff::PointShort &rhs)
{
	DWORD i1 = *(DWORD*)&lhs;
	DWORD i2 = *(DWORD*)&rhs;

	return i1 < i2;
}

bool operator<(const CollisionAdvanceRender::CollisionPair &lhs, const CollisionAdvanceRender::CollisionPair &rhs)
{
	return ff::CompareObjects(lhs, rhs) < 0;
}
