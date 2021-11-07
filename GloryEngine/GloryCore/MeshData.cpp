#include "MeshData.h"

namespace Glory
{
	MeshData::MeshData(size_t vertexCount, size_t vertexSize, const std::vector<float>& vertices, size_t indexCount, const std::vector<uint32_t>& indices, const std::vector<AttributeType>& attributes) :
		m_VertexCount(vertexCount), m_IndexCount(indexCount), m_Vertices(vertices), m_Indices(indices), m_Attributes(attributes), m_VertexSize(vertexSize)
	{
		APPEND_TYPE(MeshData);
	}

	MeshData::~MeshData()
	{
		m_Vertices.clear();
		m_Indices.clear();
		m_Attributes.clear();
	}

	const size_t MeshData::VertexCount() const
	{
		return m_VertexCount;
	}

	const size_t MeshData::IndexCount() const
	{
		return m_IndexCount;
	}

	const size_t MeshData::VertexSize() const
	{
		return m_VertexSize;
	}

	const size_t MeshData::AttributeCount() const
	{
		return m_Attributes.size();
	}

	const float* MeshData::Vertices() const
	{
		return m_Vertices.data();
	}

	const uint32_t* MeshData::Indices() const
	{
		return m_Indices.data();
	}

	const AttributeType* MeshData::AttributeTypes() const
	{
		return m_Attributes.data();
	}
}
