#include "GPUResourceManager.h"
#include <algorithm>

namespace Glory
{
	GPUResourceManager::GPUResourceManager() {}

	GPUResourceManager::~GPUResourceManager()
	{
		for (std::map<UUID, GPUResource*>::iterator it = m_IDResources.begin(); it != m_IDResources.end(); it++)
		{
			delete it->second;
		}

		std::for_each(m_pBuffers.begin(), m_pBuffers.end(), [](Buffer* pBuffer) { delete pBuffer; });
		m_pBuffers.clear();
	}

	Buffer* GPUResourceManager::CreateBuffer(uint32_t bufferSize, uint32_t usageFlag, uint32_t memoryFlags)
	{
		Buffer* pBuffer = CreateBuffer_Internal(bufferSize, usageFlag, memoryFlags);
		pBuffer->CreateBuffer();
		m_pBuffers.push_back(pBuffer);
		return pBuffer;
	}

	Mesh* GPUResourceManager::CreateMesh(MeshData* pMeshData)
	{
		Mesh* pMesh = GetResource<Mesh>(pMeshData);
		if (pMesh) return pMesh;

		uint32_t bufferSize = pMeshData->VertexCount() * pMeshData->VertexSize();
		Buffer* pVertexBuffer = CreateVertexBuffer(bufferSize);
		Buffer* pIndexBuffer = CreateIndexBuffer(bufferSize);
		pVertexBuffer->Assign(pMeshData->Vertices());
		pIndexBuffer->Assign(pMeshData->Indices());
		pMesh = CreateMesh_Internal(pMeshData);
		pMesh->SetBuffers(pVertexBuffer, pIndexBuffer);
		pMesh->CreateBindingAndAttributeData();
		m_IDResources[pMeshData->GetUUID()] = pMesh;
		return pMesh;
	}
}