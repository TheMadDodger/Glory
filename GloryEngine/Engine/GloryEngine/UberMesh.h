#pragma once
#include "GraphicsHandles.h"
#include "FragmentedBuffer.h"

#include <BitSet.h>
#include <vector>

namespace Glory
{
	class GraphicsDevice;
	class Resources;
	class MeshData;
	class Debug;

	enum class AttributeType;

	class UberMesh
	{
	public:
		GLORY_ENGINE_API UberMesh();
		GLORY_ENGINE_API UberMesh(UberMesh&& other) noexcept;
		GLORY_ENGINE_API UberMesh& operator=(UberMesh&& other) noexcept;
		GLORY_ENGINE_API ~UberMesh();

		GLORY_ENGINE_API void Initialize(std::vector<AttributeType>&& attributes, size_t vertexStride);
		GLORY_ENGINE_API bool HasMesh(UUID meshID);
		GLORY_ENGINE_API void AddMesh(UUID meshID);
		GLORY_ENGINE_API bool UpdateDirtyStates(Resources& resources);
		GLORY_ENGINE_API void UpdateMeshes(Resources& resources, GraphicsDevice* pDevice);

	private:
		struct BufferMeshID
		{
			inline bool operator==(const BufferMeshID& other)
			{
				return m_MeshID == other.m_MeshID;
			}

			inline bool operator==(const UUID& otherID)
			{
				return m_MeshID == otherID;
			}

			UUID m_MeshID;
			uint64_t m_DirtyVersion;
		};

		struct BufferMesh
		{
			uint64_t m_BaseVertex;
			uint64_t m_StartIndex;
			uint64_t m_VertexCount;
			uint64_t m_IndexCount;
		};

		void AssignMesh(BufferMesh& bufferMesh, MeshData* pMesh, GraphicsDevice* pDevice);
		void CalculateInitialSize(Resources& resources);

	private:
		FragmentedBuffer m_VertexBuffer;
		FragmentedBuffer m_IndexBuffer;
		MeshHandle m_Mesh = nullptr;

		size_t m_VertexStride = 0;

		size_t m_InitialTotalVertices = 0;
		size_t m_InitialTotalVerticesSize = 0;
		size_t m_InitialTotalIndices = 0;
		size_t m_InitialTotalIndicesSize = 0;

		std::vector<AttributeType> m_Attributes;
		std::vector<BufferMeshID> m_MeshIDs;
		std::vector<BufferMesh> m_BufferMeshes;
		Utils::BitSet m_DirtyStates;
	};
}
