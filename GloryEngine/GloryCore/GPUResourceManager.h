#pragma once
#include "UUID.h"
#include "GPUResource.h"
#include "Buffer.h"
#include <map>
#include <vector>
#include "Mesh.h"
#include "MeshData.h"

namespace Glory
{
	class GPUResourceManager
	{
	public:
		GPUResourceManager();
		virtual ~GPUResourceManager();

	public:
		Buffer* CreateBuffer(uint32_t bufferSize, uint32_t usageFlag, uint32_t memoryFlags);
		Mesh* CreateMesh(MeshData* pMeshData);

	protected: // Internal functions
		virtual Buffer* CreateVertexBuffer(uint32_t bufferSize) = 0;
		virtual Buffer* CreateIndexBuffer(uint32_t bufferSize) = 0;
		virtual Buffer* CreateBuffer_Internal(uint32_t bufferSize, uint32_t usageFlag, uint32_t memoryFlags) = 0;
		virtual Mesh* CreateMesh_Internal(MeshData* pMeshData) = 0;

	private: // Resource lookups
		template<class T>
		T* GetResource(Resource* pResource)
		{
			const UUID& uuid = pResource->GetUUID();
			auto it = m_IDResources.find(uuid);
			if (it == m_IDResources.end()) return nullptr;
			return (T*)m_IDResources[uuid];
		}

	private:
		std::vector<Buffer*> m_pBuffers;
		std::map<UUID, GPUResource*> m_IDResources;
	};
}
