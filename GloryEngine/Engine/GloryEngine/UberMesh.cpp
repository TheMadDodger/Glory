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

	UberMesh::~UberMesh()
	{
	}

	void UberMesh::AddMesh(UUID meshID)
	{
		m_StagingMeshes.push_back(meshID);
	}

	void UberMesh::PushStagingMeshes(Resources& resources, GraphicsDevice* pDevice, Debug& debug)
	{
		if (m_StagingMeshes.empty())
			return;

		std::vector<MeshData*> meshes(m_StagingMeshes.size());
		size_t totalVertices = 0;
		size_t totalIndices = 0;
		size_t vertexSize = 0;

		for (size_t i = 0; i < m_StagingMeshes.size(); ++i)
		{
			meshes[i] = resources.GetResource<MeshData>(m_StagingMeshes[i]);
			if (!meshes[i])
				continue;

			if (vertexSize == 0)
				vertexSize = meshes[i]->VertexSize();

			GLORY_ASSERT(vertexSize == meshes[i]->VertexSize(), "Mesh vertex sizes need to match for UberMesh");

			totalVertices += meshes[i]->VertexCount()*meshes[i]->VertexSize();
			totalIndices += meshes[i]->IndexCount()*sizeof(uint32_t);
		}
		m_StagingMeshes.clear();

		if (meshes.empty() || !meshes[0])
			return;

		m_BufferMeshes.reserve(m_BufferMeshes.size() + meshes.size());

		if (!m_VertexBuffer || !m_IndexBuffer || !m_Mesh)
		{
			/* Initialize the uber mesh */
			/* Double the initial counts so there is space for more meshes */
			totalVertices *= 2;
			totalIndices *= 2;

			const size_t totalVerticesSize = totalVertices*vertexSize;
			const size_t totalIndicesSize = totalIndices*sizeof(uint32_t);

			BufferHandle vertexBuffer = pDevice ?
				pDevice->CreateBuffer(totalVerticesSize, BufferType::BT_Vertex, BufferFlags::BF_ReadAndWrite) : nullptr;
			BufferHandle indexBuffer = pDevice ?
				pDevice->CreateBuffer(totalIndicesSize, BufferType::BT_Index, BufferFlags::BF_ReadAndWrite) : nullptr;

			m_VertexBuffer = FragmentedBuffer{ totalVerticesSize, vertexBuffer };
			m_IndexBuffer = FragmentedBuffer{ totalIndicesSize, indexBuffer };

			m_Mesh = pDevice ? pDevice->CreateMesh({ vertexBuffer, indexBuffer },
				totalVertices, totalIndices, vertexSize, meshes[0]->AttributeTypesVector()) : nullptr;
		}

		for (size_t i = 0; i < meshes.size(); ++i)
		{
			const auto pMesh = meshes[i];
			AddMesh_Internal(pMesh, pDevice);
		}
	}

	void UberMesh::AddMesh_Internal(MeshData* pMesh, GraphicsDevice* pDevice)
	{
		const size_t verticesSize = pMesh->VertexCount()*pMesh->VertexSize();
		const size_t indicesSize = pMesh->IndexCount()*sizeof(uint32_t);

		size_t vertexFragmentIndex, indexFragmentIndex;
		if (!m_VertexBuffer.FindFreeFragment(verticesSize, vertexFragmentIndex))
		{
			/* Resize the buffer */
			return;
		}

		if (!m_IndexBuffer.FindFreeFragment(indicesSize, indexFragmentIndex))
		{
			/* Resize the buffer */
			return;
		}

		const size_t baseVertex = m_VertexBuffer.PushData(pMesh->Vertices(), verticesSize, vertexFragmentIndex, pDevice);
		const size_t startIndex = m_IndexBuffer.PushData(pMesh->Indices(), indicesSize, indexFragmentIndex, pDevice);

		m_BufferMeshes.emplace_back(baseVertex/pMesh->VertexSize(), startIndex/sizeof(uint32_t), pMesh->IndexCount());
	}
}
