#pragma once
#include "Shapes.h"
#include "UUID.h"

#include <map>

namespace Glory
{
	class PhysicsShape {};

	struct ShapeData
	{
		const PhysicsShape* m_pShape;
		const UUID m_UUID;
	};

	class ShapeManager
	{
	public:
		ShapeManager();
		~ShapeManager();

		UUID CreateShape(const Shape& shape);
		void DestroyShape(const UUID shapeId);

		const ShapeData* GetShape(const UUID shapeId) const;

		void Clear();

	protected:
		virtual PhysicsShape* CreateShape_Internal(const Shape& shape, const UUID uuid) = 0;
		virtual void DestroyShape_Internal(ShapeData& shapeData) = 0;

	private:
		std::map<UUID, ShapeData> m_pShapes;
	};
}
