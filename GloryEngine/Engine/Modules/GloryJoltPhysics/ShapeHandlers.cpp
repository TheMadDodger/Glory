#include "Helpers.h"

#include <unordered_map>
#include <functional>

#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>

#define SHAPE_HANDLER(shape) {ShapeType::shape, shape##Creator},

namespace Glory
{
	JPH::Shape* SphereCreator(const Shape& shape);
	JPH::Shape* BoxCreator(const Shape& shape);

	std::unordered_map<ShapeType, std::function<JPH::Shape* (const Shape&)>> SHAPE_CREATORS = {
		SHAPE_HANDLER(Sphere)
		SHAPE_HANDLER(Box)
	};

	JPH::Shape* SphereCreator(const Shape& shape)
	{
		const Sphere& sphere = static_cast<const Sphere&>(shape);
		return new JPH::SphereShape(sphere.m_Radius);
	}

	JPH::Shape* BoxCreator(const Shape& shape)
	{
		const Box& box = static_cast<const Box&>(shape);
		return new JPH::BoxShape(ToJPHVec3(box.m_Extends));
	}

	JPH::Shape* GetJPHShape(const Shape& shape)
	{
		return SHAPE_CREATORS.at(shape.m_ShapeType)(shape);
	}
}