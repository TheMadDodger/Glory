#include "ShapeManager.h"

namespace Glory
{
	ShapeManager::ShapeManager()
	{
	}

	ShapeManager::~ShapeManager()
	{
	}

	UUID ShapeManager::CreateShape(const Shape& shape, bool owned)
	{
		const UUID uuid = UUID();
		PhysicsShape* pShape = CreateShape_Internal(shape, uuid);
		m_pShapes.emplace(uuid, ShapeData{ pShape, uuid, owned });
		return uuid;
	}

	void ShapeManager::DestroyShape(const UUID shapeId)
	{
	}

	const ShapeData* ShapeManager::GetShape(const UUID shapeId) const
	{
		auto& itor = m_pShapes.find(shapeId);
		if (itor == m_pShapes.end()) return nullptr;
		return &itor->second;
	}
}
