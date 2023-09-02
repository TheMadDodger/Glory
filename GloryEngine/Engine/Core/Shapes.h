#pragma once
#include <glm/glm.hpp>
#include <Reflection.h>

REFLECTABLE_ENUM_NS(Glory, ShapeType, None, Sphere, Box, Cylinder, Capsule, TaperedCapsule)

namespace Glory
{
	struct Shape
	{
	public:
		Shape(const ShapeType shapeType)
			: m_ShapeType(shapeType) {}

	public:
		const ShapeType m_ShapeType;
	};

	struct Sphere : public Shape
	{
	public:
		Sphere() : m_Radius(1.0f), Shape(ShapeType::Sphere) {}
		Sphere(float radius) : m_Radius(radius), Shape(ShapeType::Sphere) {}

	public:
		REFLECTABLE(Sphere, (float)(m_Radius));
	};

	struct Box : public Shape
	{
	public:
		Box() : m_Extends({1.0f, 1.0f, 1.0f}), Shape(ShapeType::Box) {}
		Box(const glm::vec3& extends) : m_Extends(extends), Shape(ShapeType::Box) {}

	public:
		REFLECTABLE(Box, (glm::vec3)(m_Extends));
	};

	struct Cylinder : public Shape
	{
	public:
		Cylinder() : m_HalfHeight(1.0f), m_Radius(1.0f), Shape(ShapeType::Cylinder) {}
		Cylinder(float halfHeight, float radius) : m_HalfHeight(halfHeight), m_Radius(radius), Shape(ShapeType::Cylinder) {}

	public:
		REFLECTABLE(Cylinder, (float)(m_HalfHeight), (float)(m_Radius));
	};

	struct Capsule : public Shape
	{
	public:
		Capsule() : m_HalfCylinderHeight(1.0f), m_Radius(1.0f), Shape(ShapeType::Capsule) {}
		Capsule(float halfCylinderHeight, float radius) : m_HalfCylinderHeight(halfCylinderHeight), m_Radius(radius), Shape(ShapeType::Capsule) {}

	public:
		REFLECTABLE(Capsule, (float)(m_HalfCylinderHeight), (float)(m_Radius));
	};

	struct TaperedCapsule : public Shape
	{
	public:
		TaperedCapsule()
			: m_HalfCylinderHeight(1.0f), m_TopRadius(1.0f), m_BottomRadius(1.0f), Shape(ShapeType::TaperedCapsule) {}
		TaperedCapsule(float halfCylinderHeight, float topRadius, float bottomRadius)
			: m_HalfCylinderHeight(halfCylinderHeight), m_TopRadius(topRadius), m_BottomRadius(bottomRadius), Shape(ShapeType::TaperedCapsule) {}

	public:
		REFLECTABLE(TaperedCapsule, (float)(m_HalfCylinderHeight), (float)(m_TopRadius), (float)(m_BottomRadius));
	};
}
