#pragma once
#include <vector>
#include "VertexDefinitions.h"
#include "Resource.h"

namespace Glory
{
	class MeshData : public Resource
	{
	public:
		MeshData(size_t vertexCount, size_t vertexSize, const std::vector<float>& vertices, size_t indexCount, const std::vector<uint32_t>& indices, const std::vector<AttributeType>& attributes);
		virtual ~MeshData();

		const size_t VertexCount() const;
		const size_t IndexCount() const;
		const size_t VertexSize() const;
		const size_t AttributeCount() const;
		const float* Vertices() const;
		const uint32_t* Indices() const;
		const AttributeType* AttributeTypes() const;

	private:
		friend class Mesh;
		std::vector<float> m_Vertices;
		std::vector<uint32_t> m_Indices;
		std::vector<AttributeType> m_Attributes;
		const size_t m_VertexCount;
		const size_t m_IndexCount;
		const size_t m_VertexSize;
	};
}
