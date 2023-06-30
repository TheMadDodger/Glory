#include "ShapeManager.h"

namespace Glory
{
	ShapeManager::ShapeManager()
	{
	}

	ShapeManager::~ShapeManager()
	{
		m_pShapes.clear();
	}

	UUID ShapeManager::CreateShape(const Shape& shape)
	{
		const UUID uuid = UUID();
		PhysicsShape* pShape = CreateShape_Internal(shape, uuid);
		m_pShapes.emplace(uuid, ShapeData{ pShape, uuid });
		return uuid;
	}

	void ShapeManager::DestroyShape(const UUID shapeId)
	{
		auto& itor = m_pShapes.find(shapeId);
		if (itor == m_pShapes.end()) return;
		DestroyShape_Internal(itor->second);
	}

	const ShapeData* ShapeManager::GetShape(const UUID shapeId) const
	{
		auto& itor = m_pShapes.find(shapeId);
		if (itor == m_pShapes.end()) return nullptr;
		return &itor->second;
	}

	void ShapeManager::Clear()
	{
		m_pShapes.clear();
	}
}
