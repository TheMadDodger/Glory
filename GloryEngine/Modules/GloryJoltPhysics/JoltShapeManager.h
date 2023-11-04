#pragma once
#include "Shapes.h"

#include <UUID.h>
#include <map>

namespace JPH
{
	class Shape;
}

namespace Glory
{
	struct ShapeData
	{
		ShapeData(JPH::Shape* pShape, UUID uuid);

		const JPH::Shape* m_pShape;
		const UUID m_UUID;
	};

	class JoltShapeManager
	{
	public:
		JoltShapeManager();
		~JoltShapeManager();

		UUID CreateShape(const Shape& shape);
		void DestroyShape(const UUID shapeId);

		const ShapeData* GetShape(const UUID shapeId) const;

		void Clear();

	private:
		std::map<UUID, ShapeData> m_pShapes;
	};
}
