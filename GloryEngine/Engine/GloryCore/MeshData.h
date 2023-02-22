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
		MeshData(uint32_t vertexCount, uint32_t vertexSize, const std::vector<float>& vertices, uint32_t indexCount, const std::vector<uint32_t>& indices, const std::vector<AttributeType>& attributes);
		MeshData(uint32_t vertexCount, uint32_t vertexSize, const float* vertices, uint32_t indexCount, const uint32_t* indices, const std::vector<AttributeType>& attributes);
		MeshData(uint32_t vertexCount, uint32_t vertexSize, const float* vertices, const std::vector<AttributeType>& attributes);
		virtual ~MeshData();

		const uint32_t VertexCount() const;
		const uint32_t IndexCount() const;
		const uint32_t VertexSize() const;
		const uint32_t AttributeCount() const;
		const float* Vertices() const;
		const uint32_t* Indices() const;
		const AttributeType* AttributeTypes() const;

	private:
		friend class Mesh;
		std::vector<float> m_Vertices;
		std::vector<uint32_t> m_Indices;
		std::vector<AttributeType> m_Attributes;
		const uint32_t m_VertexCount;
		const uint32_t m_IndexCount;
		const uint32_t m_VertexSize;
	};
}
