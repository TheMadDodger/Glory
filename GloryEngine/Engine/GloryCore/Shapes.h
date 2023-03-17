#pragma once
#include <glm/glm.hpp>
#include <Reflection.h>

REFLECTABLE_ENUM_NS(Glory, ShapeType, Sphere, Box)

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
}
