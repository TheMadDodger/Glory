#include "Helpers.h"

#include <unordered_map>
#include <functional>

#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/TaperedCapsuleShape.h>

#define SHAPE_HANDLER(shape) {ShapeType::shape, shape##Creator},

namespace Glory
{
	JPH::Shape* SphereCreator(const Shape& shape);
	JPH::Shape* BoxCreator(const Shape& shape);
	JPH::Shape* CylinderCreator(const Shape& shape);
	JPH::Shape* CapsuleCreator(const Shape& shape);
	JPH::Shape* TaperedCapsuleCreator(const Shape& shape);

	std::unordered_map<ShapeType, std::function<JPH::Shape* (const Shape&)>> SHAPE_CREATORS = {
		SHAPE_HANDLER(Sphere)
		SHAPE_HANDLER(Box)
		SHAPE_HANDLER(Cylinder)
		SHAPE_HANDLER(Capsule)
		SHAPE_HANDLER(TaperedCapsule)
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

	JPH::Shape* CylinderCreator(const Shape& shape)
	{
		const Cylinder& cylinder = static_cast<const Cylinder&>(shape);
		return new JPH::CylinderShape(cylinder.m_HalfHeight, cylinder.m_Radius);
	}

	JPH::Shape* CapsuleCreator(const Shape& shape)
	{
		const Capsule& capsule = static_cast<const Capsule&>(shape);
		return new JPH::CapsuleShape(capsule.m_HalfCylinderHeight, capsule.m_Radius);
	}

	JPH::Shape* TaperedCapsuleCreator(const Shape& shape)
	{
		const TaperedCapsule& capsule = static_cast<const TaperedCapsule&>(shape);
		JPH::Shape::ShapeResult result;
		return new JPH::TaperedCapsuleShape(JPH::TaperedCapsuleShapeSettings{ capsule.m_HalfCylinderHeight, capsule.m_TopRadius, capsule.m_BottomRadius }, result);
	}

	JPH::Shape* GetJPHShape(const Shape& shape)
	{
		return SHAPE_CREATORS.at(shape.m_ShapeType)(shape);
	}
}