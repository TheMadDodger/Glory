#include "MeshData.h"
#include "BinaryStream.h"

namespace Glory
{
	MeshData::MeshData()
		: m_VertexCount(0), m_IndexCount(0), m_Vertices(),
		m_Indices(), m_Attributes(), m_VertexSize(0), m_IsDirty(true)
	{
		APPEND_TYPE(MeshData);
	}

	MeshData::MeshData(uint32_t reservedVertexCount, uint32_t vertexSize, std::vector<AttributeType>&& attributes):
		m_VertexCount(0), m_IndexCount(0), m_Vertices(), m_Indices(),
		m_Attributes(std::move(attributes)), m_VertexSize(vertexSize), m_IsDirty(true)
	{
		m_Vertices.reserve(reservedVertexCount*vertexSize);
		m_Indices.reserve(reservedVertexCount);
		APPEND_TYPE(MeshData);
	}

	MeshData::MeshData(uint32_t vertexCount, uint32_t vertexSize, const std::vector<float>& vertices, uint32_t indexCount, const std::vector<uint32_t>& indices, const std::vector<AttributeType>& attributes) :
		m_VertexCount(vertexCount), m_IndexCount(indexCount), m_Vertices(vertices),
		m_Indices(indices), m_Attributes(attributes), m_VertexSize(vertexSize), m_IsDirty(true)
	{
		APPEND_TYPE(MeshData);
	}

	MeshData::MeshData(uint32_t vertexCount, uint32_t vertexSize, const float* vertices, uint32_t indexCount, const uint32_t* indices, const std::vector<AttributeType>& attributes) :
		m_VertexCount(vertexCount), m_IndexCount(indexCount), m_Vertices(std::vector<float>(vertexCount)),
		m_Indices(std::vector<uint32_t>(indexCount)), m_Attributes(attributes), m_VertexSize(vertexSize), m_IsDirty(true)
	{
		memcpy(&m_Vertices[0], vertices, sizeof(float) * vertexCount);
		memcpy(&m_Indices[0], indices, sizeof(uint32_t) * indexCount);
		APPEND_TYPE(MeshData);
	}

	MeshData::MeshData(uint32_t vertexCount, uint32_t vertexSize, const float* vertices, const std::vector<AttributeType>& attributes) :
		m_VertexCount(vertexCount), m_IndexCount(0), m_Vertices(std::vector<float>(vertexCount)),
		m_Indices(), m_Attributes(attributes), m_VertexSize(vertexSize), m_IsDirty(true)
	{
		memcpy(&m_Vertices[0], vertices, sizeof(float) * vertexCount);
		APPEND_TYPE(MeshData);
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

	float* MeshData::Vertices()
	{
		return m_Vertices.data();
	}

	const float* MeshData::Vertices() const
	{
		return m_Vertices.data();
	}

	uint32_t* MeshData::Indices()
	{
		return m_Indices.data();
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
		m_Vertices.resize(m_VertexCount*m_VertexSize/sizeof(float));
		container.Read(m_Vertices.data(), sizeof(float)*m_Vertices.size());
		m_Indices.resize(m_IndexCount);
		container.Read(m_Indices.data(), sizeof(uint32_t)*m_Indices.size());
	}

	uint32_t MeshData::AddVertex(const float* vertex)
	{
		const size_t writePos = m_Vertices.size();
		m_Vertices.resize(writePos + m_VertexSize/sizeof(float));
		std::memcpy(&m_Vertices[writePos], vertex, m_VertexSize);
		++m_VertexCount;
		return m_VertexCount - 1;
	}

	void MeshData::AddFace(uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3)
	{
		/* Triangle 1 */
		m_Indices.push_back(v0);
		m_Indices.push_back(v1);
		m_Indices.push_back(v2);

		/* Triangle 2 */
		m_Indices.push_back(v0);
		m_Indices.push_back(v2);
		m_Indices.push_back(v3);

		m_IndexCount += 6;

		m_IsDirty = true;
	}

	void MeshData::ClearVertices()
	{
		m_Vertices.clear();
		m_VertexCount = 0;
		m_IsDirty = true;
	}

	void MeshData::ClearIndices()
	{
		m_Indices.clear();
		m_IndexCount = 0;
		m_IsDirty = true;
	}

	bool MeshData::IsDirty() const
	{
		return m_IsDirty;
	}

	void MeshData::SetDirty(bool dirty)
	{
		m_IsDirty = dirty;
	}

	void MeshData::Merge(MeshData* pOther)
	{
		m_VertexCount += pOther->m_VertexCount;
		m_IndexCount += pOther->m_IndexCount;
		const size_t vertexStart = m_Vertices.size();
		const size_t indexStart = m_Indices.size();
		m_Vertices.resize(m_Vertices.size() + pOther->m_Vertices.size());
		m_Indices.resize(m_Indices.size() + pOther->m_Indices.size());
		std::memcpy(&m_Vertices[vertexStart], pOther->m_Vertices.data(), pOther->m_Vertices.size()*sizeof(float));
		std::memcpy(&m_Indices[indexStart], pOther->m_Indices.data(), pOther->m_Indices.size()*sizeof(uint32_t));
		m_IsDirty = true;
	}

	void MeshData::AddBoundingBox(const glm::vec3& min, const glm::vec3& max)
	{
		const glm::vec3 halfExtends = (max - min)/2.0f;
		m_BoundingBox.m_HalfExtends = glm::vec4(halfExtends, 1.0f);
		m_BoundingBox.m_Center = glm::vec4(min + halfExtends, 1.0f);
	}

	const BoundingBox& MeshData::GetBoundingBox() const
	{
		return m_BoundingBox;
	}
}
