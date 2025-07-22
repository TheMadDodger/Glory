#pragma once
#include "VertexDefinitions.h"
#include "Resource.h"

#include <vector>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace Glory
{
	struct BoundingBox
	{
		BoundingBox() : m_Center{}, m_HalfExtends{} {}

		glm::vec4 m_Center;
		glm::vec4 m_HalfExtends;
	};

	class MeshData : public Resource
	{
	public:
		MeshData();
		MeshData(uint32_t reservedVertexCount, uint32_t vertexSize, std::vector<AttributeType>&& attributes);
		MeshData(uint32_t vertexCount, uint32_t vertexSize, const std::vector<float>& vertices, uint32_t indexCount, const std::vector<uint32_t>& indices, const std::vector<AttributeType>& attributes);
		MeshData(uint32_t vertexCount, uint32_t vertexSize, const float* vertices, uint32_t indexCount, const uint32_t* indices, const std::vector<AttributeType>& attributes);
		MeshData(uint32_t vertexCount, uint32_t vertexSize, const float* vertices, const std::vector<AttributeType>& attributes);
		virtual ~MeshData();

		const uint32_t VertexCount() const;
		const uint32_t IndexCount() const;
		const uint32_t VertexSize() const;
		const uint32_t AttributeCount() const;
		float* Vertices();
		const float* Vertices() const;
		uint32_t* Indices();
		const uint32_t* Indices() const;
		const AttributeType* AttributeTypes() const;

		void Serialize(BinaryStream& container) const override;
		void Deserialize(BinaryStream& container) override;

		uint32_t AddVertex(const float* vertex);
		void AddFace(uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3);
		void ClearVertices();
		void ClearIndices();

		bool IsDirty() const;
		void SetDirty(bool dirty);

		void Merge(MeshData* pOther);

		void AddBoundingBox(const glm::vec3& min, const glm::vec3& max);
		const BoundingBox& GetBoundingBox() const;

	private:
		virtual void References(Engine*, std::vector<UUID>&) const override {}

	private:
		friend class Mesh;
		std::vector<float> m_Vertices;
		std::vector<uint32_t> m_Indices;
		std::vector<AttributeType> m_Attributes;
		uint32_t m_VertexCount;
		uint32_t m_IndexCount;
		uint32_t m_VertexSize;
		bool m_IsDirty;

		BoundingBox m_BoundingBox;
	};
}
