#include "BoundingBox.h"
#include "Components.h"

#include "Engine.h"
#include "AssetManager.h"

#include <glm/gtx/matrix_decompose.hpp>

#include <algorithm>

namespace Glory
{
	AABB::AABB(const glm::vec3& origin):
		m_Min(origin), m_Max(origin)
	{
	}

	AABB::AABB(const glm::vec3& min, const glm::vec3& max):
		m_Min(min), m_Max(max)
	{
	}

	void AABB::Combine(const BoundingBox& bounds, const glm::mat4& transform)
	{
		glm::vec4 points[8];
		points[0] = bounds.m_Center + glm::vec4(-bounds.m_HalfExtends.x, bounds.m_HalfExtends.y, -bounds.m_HalfExtends.z, 0.0f);
		points[1] = bounds.m_Center + glm::vec4(bounds.m_HalfExtends.x, bounds.m_HalfExtends.y, -bounds.m_HalfExtends.z, 0.0f);
		points[2] = bounds.m_Center + glm::vec4(bounds.m_HalfExtends.x, bounds.m_HalfExtends.y, bounds.m_HalfExtends.z, 0.0f);
		points[3] = bounds.m_Center + glm::vec4(-bounds.m_HalfExtends.x, bounds.m_HalfExtends.y, bounds.m_HalfExtends.z, 0.0f);
		points[4] = bounds.m_Center + glm::vec4(-bounds.m_HalfExtends.x, -bounds.m_HalfExtends.y, -bounds.m_HalfExtends.z, 0.0f);
		points[5] = bounds.m_Center + glm::vec4(bounds.m_HalfExtends.x, -bounds.m_HalfExtends.y, -bounds.m_HalfExtends.z, 0.0f);
		points[6] = bounds.m_Center + glm::vec4(bounds.m_HalfExtends.x, -bounds.m_HalfExtends.y, bounds.m_HalfExtends.z, 0.0f);
		points[7] = bounds.m_Center + glm::vec4(-bounds.m_HalfExtends.x, -bounds.m_HalfExtends.y, bounds.m_HalfExtends.z, 0.0f);

		for (size_t i = 0; i < 8; ++i)
		{
			points[i] = transform*points[i];
			m_Min = glm::min(m_Min, glm::vec3(points[i]));
			m_Max = glm::max(m_Max, glm::vec3(points[i]));
		}
	}

	BoundingSphere::BoundingSphere(const glm::vec3& center, float radius) :
		m_CenterAndRadius(center, radius)
	{
	}

	void BoundingSphere::Combine(const BoundingSphere& bounds, const glm::mat4& transform)
	{
		glm::vec3 scale;
		glm::quat rotation;
		glm::vec3 translation;
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(transform, scale, rotation, translation, skew, perspective);
		const float radiusScale = glm::max(glm::max(scale.x, scale.y), scale.z);
		const glm::vec3 transformedCenter = transform*glm::vec4{ bounds.m_Center, 1.0f };
		const float scaledRadius = bounds.m_Radius*radiusScale;

		if (m_Radius == 0.0f)
		{
			m_Center = transformedCenter;
			m_Radius = scaledRadius;
			return;
		}

		if (m_Center == transformedCenter)
		{
			m_Radius = std::max(m_Radius, scaledRadius);
			return;
		}

		/* Find midpoint between centers */
		const glm::vec3 toOtherCenter = transformedCenter - m_Center;
		const glm::vec3 edgeToEdge = toOtherCenter + glm::normalize(toOtherCenter)*(m_Radius+scaledRadius);
		m_Center = m_Center - glm::normalize(toOtherCenter)*m_Radius + edgeToEdge/2.0f;
		m_Radius = glm::length(edgeToEdge)/2.0f;
	}

	static void CollectAABBChild(Entity entity, AABB& boundingBox, Engine* pEngine)
	{
		if (entity.HasComponent<MeshRenderer>())
		{
			MeshRenderer& mesh = entity.GetComponent<MeshRenderer>();
			const glm::mat4& world = entity.GetComponent<Transform>().MatTransform;
			Resource* pResource = pEngine->GetAssetManager().FindResource(mesh.m_Mesh.AssetUUID());
			if (pResource)
			{
				MeshData* pMesh = static_cast<MeshData*>(pResource);
				boundingBox.Combine(pMesh->GetBoundingBox(), world);
			}
		}

		for (size_t i = 0; i < entity.ChildCount(); i++)
			CollectAABBChild(entity.ChildEntity(i), boundingBox, pEngine);
	}

	AABB GenerateAABB(Entity entity, Engine* pEngine, const glm::vec3& origin)
	{
		AABB boundingBox{ origin };
		CollectAABBChild(entity, boundingBox, pEngine);
		return boundingBox;
	}

	static void CollectBoundingSphereChild(Entity entity, BoundingSphere& boundingSphere, Engine* pEngine)
	{
		if (entity.HasComponent<MeshRenderer>())
		{
			MeshRenderer& mesh = entity.GetComponent<MeshRenderer>();
			const glm::mat4& world = entity.GetComponent<Transform>().MatTransform;
			Resource* pResource = pEngine->GetAssetManager().FindResource(mesh.m_Mesh.AssetUUID());
			if (pResource)
			{
				MeshData* pMesh = static_cast<MeshData*>(pResource);
				boundingSphere.Combine(pMesh->GetBoundingSphere(), world);
			}
		}

		for (size_t i = 0; i < entity.ChildCount(); ++i)
			CollectBoundingSphereChild(entity.ChildEntity(i), boundingSphere, pEngine);
	}

	BoundingSphere GenerateBoundingSphere(Entity entity, Engine* pEngine, const glm::vec3& defaultCenter, float defaultRadius)
	{
		BoundingSphere boundingSphere{ {}, 0.0f };
		CollectBoundingSphereChild(entity, boundingSphere, pEngine);

		if (boundingSphere.m_Radius == 0.0f)
		{
			boundingSphere.m_Center = defaultCenter;
			boundingSphere.m_Radius = defaultRadius;
		}
		return boundingSphere;
	}
}