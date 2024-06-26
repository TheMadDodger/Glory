#pragma once
#include <vector>
#include "VertexDefinitions.h"
#include "Resource.h"

namespace Glory
{
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

	private:
		friend class Mesh;
		std::vector<float> m_Vertices;
		std::vector<uint32_t> m_Indices;
		std::vector<AttributeType> m_Attributes;
		uint32_t m_VertexCount;
		uint32_t m_IndexCount;
		uint32_t m_VertexSize;
	};
}
