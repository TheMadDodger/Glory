#pragma once
#include "Shapes.h"

#include <UUID.h>
#include <map>
#include <Glory.h>

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

		GLORY_API UUID CreateShape(const Shape& shape);
		GLORY_API void DestroyShape(const UUID shapeId);

		GLORY_API const ShapeData* GetShape(const UUID shapeId) const;

		GLORY_API void Clear();

	private:
		std::map<UUID, ShapeData> m_pShapes;
	};
}
