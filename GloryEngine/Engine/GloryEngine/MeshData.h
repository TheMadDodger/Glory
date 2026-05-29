#pragma once
#include "VertexDefinitions.h"
#include "Resource.h"
#include "BoundingBox.h"

#include <engine_visibility.h>

#include <glm/glm.hpp>

#include <vector>

namespace Glory
{
	class MeshData : public Resource
	{
	public:
		GLORY_ENGINE_API MeshData();
		GLORY_ENGINE_API MeshData(uint32_t reservedVertexCount, uint32_t vertexSize, std::vector<AttributeType>&& attributes);
		GLORY_ENGINE_API MeshData(uint32_t vertexCount, uint32_t vertexSize, const std::vector<float>& vertices,
			uint32_t indexCount, const std::vector<uint32_t>& indices, const std::vector<AttributeType>& attributes);
		GLORY_ENGINE_API MeshData(uint32_t vertexCount, uint32_t vertexSize, const float* vertices, uint32_t indexCount,
			const uint32_t* indices, const std::vector<AttributeType>& attributes);
		GLORY_ENGINE_API MeshData(uint32_t vertexCount, uint32_t vertexSize, const float* vertices, const std::vector<AttributeType>& attributes);

		GLORY_ENGINE_API MeshData(MeshData&&) noexcept = default;
		GLORY_ENGINE_API MeshData& operator=(MeshData&&) noexcept = default;

		GLORY_ENGINE_API virtual ~MeshData();

		GLORY_ENGINE_API const uint32_t VertexCount() const;
		GLORY_ENGINE_API const uint32_t IndexCount() const;
		GLORY_ENGINE_API const uint32_t VertexSize() const;
		GLORY_ENGINE_API const uint32_t AttributeCount() const;
		GLORY_ENGINE_API float* Vertices();
		GLORY_ENGINE_API const float* Vertices() const;
		GLORY_ENGINE_API uint32_t* Indices();
		GLORY_ENGINE_API const uint32_t* Indices() const;
		GLORY_ENGINE_API const AttributeType* AttributeTypes() const;
		GLORY_ENGINE_API const std::vector<AttributeType>& AttributeTypesVector() const;

		GLORY_ENGINE_API void Serialize(Utils::BinaryStream& container) const override;
		GLORY_ENGINE_API void Deserialize(Utils::BinaryStream& container) override;

		GLORY_ENGINE_API uint32_t AddVertex(const float* vertex);
		GLORY_ENGINE_API void AddFace(uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3);
		GLORY_ENGINE_API void ClearVertices();
		GLORY_ENGINE_API void ClearIndices();

		GLORY_ENGINE_API void Merge(MeshData* pOther);

		GLORY_ENGINE_API void AddBoundingBox(const glm::vec3& min, const glm::vec3& max);
		GLORY_ENGINE_API void AddBoundingSphere(const glm::vec3& center, float radius);
		GLORY_ENGINE_API const BoundingBox& GetBoundingBox() const;
		GLORY_ENGINE_API const BoundingSphere& GetBoundingSphere() const;

	private:
		virtual void References(IEngine*, std::vector<UUID>&) const override {}

	private:
		std::vector<float> m_Vertices;
		std::vector<uint32_t> m_Indices;
		std::vector<AttributeType> m_Attributes;
		uint32_t m_VertexCount;
		uint32_t m_IndexCount;
		uint32_t m_VertexSize;
		BoundingBox m_BoundingBox;
		BoundingSphere m_BoundingSphere;
	};
}
