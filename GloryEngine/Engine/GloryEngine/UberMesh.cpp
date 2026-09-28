#include "UberMesh.h"
#include "Resources.h"
#include "MeshData.h"
#include "GraphicsDevice.h"
#include "Debug.h"

namespace Glory
{
	UberMesh::UberMesh()
	{
	}

	UberMesh::UberMesh(UberMesh&& other) noexcept :
		m_VertexBuffer(std::move(other.m_VertexBuffer)),
		m_IndexBuffer(std::move(other.m_IndexBuffer)),
		m_Mesh(other.m_Mesh), m_VertexStride(other.m_VertexStride),
		m_Attributes(std::move(other.m_Attributes)),
		m_MeshIDs(std::move(other.m_MeshIDs)),
		m_BufferMeshes(std::move(other.m_BufferMeshes))
	{
	}

	UberMesh& UberMesh::operator=(UberMesh&& other) noexcept
	{
		m_VertexBuffer = std::move(other.m_VertexBuffer);
		m_IndexBuffer = std::move(other.m_IndexBuffer);
		m_Mesh = other.m_Mesh;
		m_VertexStride = other.m_VertexStride;
		m_Attributes = std::move(other.m_Attributes);
		m_MeshIDs = std::move(other.m_MeshIDs);
		m_BufferMeshes = std::move(other.m_BufferMeshes);
		return *this;
	}

	UberMesh::~UberMesh()
	{
	}

	void UberMesh::Initialize(std::vector<AttributeType>&& attributes, size_t vertexStride)
	{
		m_Attributes = std::move(attributes);
		m_VertexStride = vertexStride;
	}

	bool UberMesh::HasMesh(UUID meshID)
	{
		return std::find(m_MeshIDs.begin(), m_MeshIDs.end(), meshID) != m_MeshIDs.end();
	}

	void UberMesh::AddMesh(UUID meshID)
	{
		const size_t index = m_MeshIDs.size();
		m_MeshIDs.emplace_back(meshID, 0ull);
		m_BufferMeshes.emplace_back(0ull, 0ull, 0ull, 0ull);
		m_DirtyStates.Reserve(m_MeshIDs.size());
		m_DirtyStates.Set(index);
	}

	bool UberMesh::UpdateDirtyStates(Resources& resources)
	{
		for (size_t i = 0; i < m_BufferMeshes.size(); ++i)
		{
			auto& meshID = m_MeshIDs[i];

			MeshData* pMesh = resources.GetResource<MeshData>(meshID.m_MeshID);

			/* Can't update when a mesh isn't loaded yet */
			if (!pMesh)
				return false;

			if (m_Attributes.empty())
			{
				m_Attributes = pMesh->AttributeTypesVector();
				m_VertexStride = pMesh->VertexSize();
			}

			GLORY_ASSERT(m_VertexStride == pMesh->VertexSize(), "Mesh vertex sizes need to match for UberMesh");

			if (pMesh->IsDirty(meshID.m_DirtyVersion))
				m_DirtyStates.Set(i);

			meshID.m_DirtyVersion = pMesh->DirtyVersion();
		}

		return m_DirtyStates.HasAnySet();
	}

	void UberMesh::UpdateMeshes(Resources& resources, GraphicsDevice* pDevice)
	{
		if (m_MeshIDs.empty() || !m_DirtyStates.HasAnySet())
			return;

		if (!m_VertexBuffer || !m_IndexBuffer || !m_Mesh)
		{
			/* Initialize the uber mesh */
			CalculateInitialSize(resources);

			BufferHandle vertexBuffer = pDevice ?
				pDevice->CreateBuffer(m_InitialTotalVerticesSize, BufferType::BT_Vertex, BufferFlags(BF_Coherent | BF_Write)) : nullptr;
			BufferHandle indexBuffer = pDevice ?
				pDevice->CreateBuffer(m_InitialTotalIndicesSize, BufferType::BT_Index, BufferFlags(BF_Coherent | BF_Write)) : nullptr;

			m_VertexBuffer = FragmentedBuffer{ m_InitialTotalVerticesSize, vertexBuffer };
			m_IndexBuffer = FragmentedBuffer{ m_InitialTotalIndicesSize, indexBuffer };

			m_Mesh = pDevice ? pDevice->CreateMesh({ vertexBuffer, indexBuffer },
				m_InitialTotalVertices, m_InitialTotalIndices, m_VertexStride, m_Attributes) : nullptr;
		}

		for (size_t i = 0; i < m_BufferMeshes.size(); ++i)
		{
			if (!m_DirtyStates.IsSet(i))
				continue;

			const auto& meshID = m_MeshIDs[i];
			auto& bufferMesh = m_BufferMeshes[i];
			if (bufferMesh.m_IndexCount > 0 || bufferMesh.m_VertexCount > 0)
			{
				if (bufferMesh.m_IndexCount > 0)
					m_IndexBuffer.FreeData(bufferMesh.m_StartIndex*sizeof(uint32_t),
						bufferMesh.m_IndexCount*sizeof(uint32_t));

				if (bufferMesh.m_VertexCount > 0)
					m_VertexBuffer.FreeData(bufferMesh.m_BaseVertex*m_VertexStride,
						bufferMesh.m_VertexCount*m_VertexStride);
			}

			MeshData* pMesh = resources.GetResource<MeshData>(meshID.m_MeshID);
			if (!pMesh)
				continue;

			AssignMesh(bufferMesh, pMesh, pDevice);
			m_DirtyStates.UnSet(i);
		}
	}

	void UberMesh::AssignMesh(UberMesh::BufferMesh& bufferMesh, MeshData* pMesh, GraphicsDevice* pDevice)
	{
		const size_t verticesSize = pMesh->VertexCount()*pMesh->VertexSize();
		const size_t indicesSize = pMesh->IndexCount()*sizeof(uint32_t);

		size_t vertexFragmentIndex, indexFragmentIndex;
		if (!m_VertexBuffer.FindFreeFragment(verticesSize, vertexFragmentIndex))
		{
			/* Resize the buffer */
			GLORY_ASSERT_UNREACHABLE_CODE();
			return;
		}

		if (!m_IndexBuffer.FindFreeFragment(indicesSize, indexFragmentIndex))
		{
			/* Resize the buffer */
			GLORY_ASSERT_UNREACHABLE_CODE();
			return;
		}

		const size_t baseVertexByte = m_VertexBuffer.PushData(pMesh->Vertices(), verticesSize, vertexFragmentIndex, pDevice);
		const size_t startIndexByte = m_IndexBuffer.PushData(pMesh->Indices(), indicesSize, indexFragmentIndex, pDevice);

		bufferMesh.m_BaseVertex = baseVertexByte/m_VertexStride;
		bufferMesh.m_StartIndex = startIndexByte/sizeof(uint32_t);
		bufferMesh.m_IndexCount = pMesh->IndexCount();
		bufferMesh.m_VertexCount = pMesh->VertexCount();
	}

	void UberMesh::CalculateInitialSize(Resources& resources)
	{
		/* Double the initial counts so there is space for more meshes */
		size_t totalVertices = 0;
		size_t totalIndices = 0;

		for (size_t i = 0; i < m_BufferMeshes.size(); ++i)
		{
			const auto& meshID = m_MeshIDs[i];

			MeshData* pMesh = resources.GetResource<MeshData>(meshID.m_MeshID);

			if (m_Attributes.empty())
			{
				m_Attributes = pMesh->AttributeTypesVector();
				m_VertexStride = pMesh->VertexSize();
			}

			/* Can't update when a mesh isn't loaded yet */
			if (!pMesh)
				return;

			GLORY_ASSERT(m_VertexStride == pMesh->VertexSize(), "Mesh vertex sizes need to match for UberMesh");

			totalVertices += pMesh->VertexCount()*pMesh->VertexSize();
			totalIndices += pMesh->IndexCount()*sizeof(uint32_t);
		}

		totalVertices *= 2;
		totalIndices *= 2;

		m_InitialTotalVertices = totalVertices;
		m_InitialTotalVerticesSize = totalVertices*m_VertexStride;
		m_InitialTotalIndices = totalIndices;
		m_InitialTotalIndicesSize = totalIndices*sizeof(uint32_t);
	}
}
