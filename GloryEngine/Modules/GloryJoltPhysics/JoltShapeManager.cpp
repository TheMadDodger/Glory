#include "JoltShapeManager.h"
#include "Helpers.h"

namespace Glory
{
	JoltShapeManager::JoltShapeManager()
	{
	}

	JoltShapeManager::~JoltShapeManager()
	{
		m_pShapes.clear();
	}

	UUID JoltShapeManager::CreateShape(const Shape& shape)
	{
		const UUID uuid = UUID();
		JPH::Shape* pShape = GetJPHShape(shape);
		m_pShapes.emplace(uuid, ShapeData{ pShape, uuid });
		return uuid;
	}

	void JoltShapeManager::DestroyShape(const UUID)
	{
	}

	const ShapeData* JoltShapeManager::GetShape(const UUID shapeId) const
	{
		auto& itor = m_pShapes.find(shapeId);
		if (itor == m_pShapes.end()) return nullptr;
		return &itor->second;
	}

	void JoltShapeManager::Clear()
	{
		m_pShapes.clear();
	}

	ShapeData::ShapeData(JPH::Shape* pShape, UUID uuid):
		m_pShape(pShape), m_UUID(uuid)
	{
	}
}
