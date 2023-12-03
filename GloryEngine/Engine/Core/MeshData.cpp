#include "MeshData.h"
#include "BinaryStream.h"

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
		return (uint32_t)m_Attributes.size();
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

	void MeshData::Serialize(BinaryStream& container) const
	{
		container.Write(m_VertexCount);
		container.Write(m_IndexCount);
		container.Write(m_VertexSize);
		container.Write(m_Attributes.size());
		container.Write(reinterpret_cast<const char*>(m_Attributes.data()), sizeof(AttributeType)*m_Attributes.size());
		container.Write(reinterpret_cast<const char*>(m_Vertices.data()), sizeof(float)*m_Vertices.size());
		container.Write(reinterpret_cast<const char*>(m_Indices.data()), sizeof(uint32_t)*m_Indices.size());
	}

	void MeshData::Deserialize(BinaryStream& container)
	{
		container.Read(m_VertexCount);
		container.Read(m_IndexCount);
		container.Read(m_VertexSize);
		size_t attributeCount;
		container.Read(attributeCount);
		m_Attributes.resize(attributeCount);
		container.Read(m_Attributes.data(), sizeof(AttributeType)*m_Attributes.size());
		m_Vertices.resize(m_VertexCount);
		container.Read(m_Vertices.data(), sizeof(float)*m_Vertices.size());
		m_Indices.resize(m_IndexCount);
		container.Read(m_Indices.data(), sizeof(uint32_t)*m_Indices.size());
	}
}
