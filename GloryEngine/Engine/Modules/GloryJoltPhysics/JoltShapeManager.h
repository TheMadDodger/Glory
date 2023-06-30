#pragma once
#include <ShapeManager.h>

namespace Glory
{
	class JoltShapeManager : public ShapeManager
	{
	private:
		virtual PhysicsShape* CreateShape_Internal(const Shape& shape, const UUID uuid) override;
		virtual void DestroyShape_Internal(ShapeData&) override {};
	};
}
