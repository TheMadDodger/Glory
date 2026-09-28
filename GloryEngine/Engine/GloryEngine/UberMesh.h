#pragma once
#include "GraphicsHandles.h"
#include "FragmentedBuffer.h"

#include <vector>

namespace Glory
{
	class GraphicsDevice;
	class Resources;
	class MeshData;
	class Debug;

	class UberMesh
	{
	public:
		UberMesh();
		~UberMesh();

		void AddMesh(UUID meshID);
		void PushStagingMeshes(Resources& resources, GraphicsDevice* pDevice, Debug& debug);

	private:
		void AddMesh_Internal(MeshData* pMesh, GraphicsDevice* pDevice);

	private:
		std::vector<UUID> m_StagingMeshes;

		FragmentedBuffer m_VertexBuffer;
		FragmentedBuffer m_IndexBuffer;
		MeshHandle m_Mesh = nullptr;

		struct BufferMesh
		{
			size_t m_BaseVertex;
			size_t m_StartIndex;
			size_t m_IndexCount;
		};

		std::vector<BufferMesh> m_BufferMeshes;
	};
}
