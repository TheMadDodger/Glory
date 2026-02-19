#pragma once
#include "Entity.h"

#include <glm/glm.hpp>

namespace Glory
{
	class Engine;

	struct BoundingBox
	{
		BoundingBox() :
			m_Center(), m_HalfExtends() {
		}

		BoundingBox(const glm::vec4& center, const glm::vec4 halfExtends) :
			m_Center(center), m_HalfExtends(halfExtends) {
		}

		glm::vec4 m_Center;
		glm::vec4 m_HalfExtends;
	};

	struct AABB
	{
		AABB(const glm::vec3& origin={});
		AABB(const glm::vec3& min, const glm::vec3& max);

		glm::vec3 m_Min;
		glm::vec3 m_Max;

		void Combine(const BoundingBox& bounds, const glm::mat4& transform);
	};

	struct BoundingSphere
	{
		BoundingSphere(const glm::vec3& center={}, float radius=1.0f);

		union
		{
			glm::vec4 m_CenterAndRadius;
			struct
			{
				glm::vec3 m_Center;
				float m_Radius;
			};
		};

		void Combine(const BoundingSphere& bounds, const glm::mat4& transform);
	};

	AABB GenerateAABB(Entity entity, Engine* pEngine, const glm::vec3& origin={});
	BoundingSphere GenerateBoundingSphere(Entity entity, Engine* pEngine, const glm::vec3& defaultCenter={}, float defaultRadius=1.0f);
}
