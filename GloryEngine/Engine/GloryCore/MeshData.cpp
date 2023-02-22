#include "MeshData.h"

namespace Glory
{
	MeshData::MeshData()
		: m_VertexCount(0), m_IndexCount(0), m_Vertices(), m_Indices(), m_Attributes(), m_VertexSize(0)
	{
		APPEND_TYPE(MeshData);
	}

	MeshData::MeshData(uint32_t vertexCount, uint32_t vertexSize, const std::vector<float>& vertices, uint32_t indexCount, const std::vector<uint32_t>& indices, const std::vector<AttributeType>& attributes) :
		m_VertexCount(vertexCount), m_IndexCount(indexCount), m_Vertices(vertices), m_Indices(indices), m_Attributes(attributes), m_VertexSize(vertexSize)
	{
		APPEND_TYPE(MeshData);
	}

	MeshData::MeshData(uint32_t vertexCount, uint32_t vertexSize, const float* vertices, uint32_t indexCount, const uint32_t* indices, const std::vector<AttributeType>& attributes) :
		m_VertexCount(vertexCount), m_IndexCount(indexCount), m_Vertices(std::vector<float>(vertexCount)), m_Indices(std::vector<uint32_t>(indexCount)), m_Attributes(attributes), m_VertexSize(vertexSize)
	{
		memcpy(&m_Vertices[0], vertices, sizeof(float) * vertexCount);
		memcpy(&m_Indices[0], indices, sizeof(uint32_t) * indexCount);
	}

	MeshData::MeshData(uint32_t vertexCount, uint32_t vertexSize, const float* vertices, const std::vector<AttributeType>& attributes) :
		m_VertexCount(vertexCount), m_IndexCount(0), m_Vertices(std::vector<float>(vertexCount)), m_Indices(), m_Attributes(attributes), m_VertexSize(vertexSize)
	{
		memcpy(&m_Vertices[0], vertices, sizeof(float) * vertexCount);
	}

	MeshData::~MeshData()
	{
		m_Vertices.clear();
		m_Indices.clear();
		m_Attributes.clear();
	}

	const uint32_t MeshData::VertexCount() const
	{
		return m_VertexCount;
	}

	const uint32_t MeshData::IndexCount() const
	{
		return m_IndexCount;
	}

	const uint32_t MeshData::VertexSize() const
	{
		return m_VertexSize;
	}

	const uint32_t MeshData::AttributeCount() const
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
