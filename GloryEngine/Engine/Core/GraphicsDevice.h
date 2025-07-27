#pragma once
#include "VertexDefinitions.h"
#include "GraphicsEnums.h"
#include "UUID.h"

namespace Glory
{
	class Module;
	class Debug;

	class Resource;
	class MeshData;
	class TextureData;
	class FileData;
	class MaterialData;
	class CubemapData;
	class TextureAtlas;
	class PipelineData;

	struct TextureCreateInfo;
	struct RenderTextureCreateInfo;

	typedef struct UUID BufferHandle;
	typedef struct UUID MeshHandle;

	enum BufferType
	{
		BT_TransferRead,
		BT_TransferWrite,
		BT_Vertex,
		BT_Index
	};

	template<class T>
	struct GraphicsResources
	{
	public:
		void Clear()
		{
			m_IDs.clear();
			m_Resources.clear();
		}

		T& Emplace(UUID id, T&& resource)
		{
			m_IDs.emplace_back(id);
			return m_Resources.emplace_back(std::move(resource));
		}

		template <class... _Valty>
		T& Emplace(UUID id, _Valty&&... _Val)
		{
			m_IDs.emplace_back(id);
			return m_Resources.emplace_back(forward<_Valty>(_Val)...);
		}

		T* Find(UUID id)
		{
			auto iter = std::find(m_IDs.begin(), m_IDs.end(), id);
			if (iter == m_IDs.end()) return nullptr;
			const size_t index = iter - m_IDs.begin();
			return &m_Resources[index];
		}

		void Erase(UUID id)
		{
			auto iter = std::find(m_IDs.begin(), m_IDs.end(), id);
			if (iter == m_IDs.end()) return;
			const size_t index = iter - m_IDs.begin();
			m_IDs.erase(iter);
			m_Resources.erase(m_Resources.begin() + index);
		}

	private:
		std::vector<UUID> m_IDs;
		std::vector<T> m_Resources;
	};

	class GraphicsDevice
	{
	public:
		GraphicsDevice(Module* pModule);
		virtual ~GraphicsDevice();

	public: /* Resource mamagement */
		
		/* Buffer */

		/**
		 * @brief Create a buffer on this device
		 * @param bufferSize Size of the buffer in bytes
		 * @param type Type of the buffer
		 */
		virtual BufferHandle CreateBuffer(size_t bufferSize, BufferType type) = 0;

		virtual void AssignBuffer(BufferHandle handle, const void* data) = 0;
		virtual void AssignBuffer(BufferHandle handle, const void* data, uint32_t size) = 0;
		virtual void AssignBuffer(BufferHandle handle, const void* data, uint32_t offset, uint32_t size) = 0;

		/* Mesh */

		/** @brief Create a mesh on this device */
		MeshHandle CreateMesh(MeshData* pMeshData);
		virtual MeshHandle CreateMesh(std::vector<BufferHandle>&& buffers, uint32_t vertexCount,
			uint32_t indexCount, uint32_t stride, PrimitiveType primitiveType,
			const std::vector<AttributeType>& attributeTypes) = 0;

		/* Free memory */

		/** @brief Free a buffer from device memory */
		virtual void FreeBuffer(BufferHandle& handle) = 0;
		/** @brief Free a mesh from device memory, this also frees any buffers the mesh owns */
		virtual void FreeMesh(MeshHandle& handle) = 0;

	protected:
		Debug& Debug();

	protected:
		Module* m_pModule;
	};
}
