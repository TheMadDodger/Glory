#pragma once
#include "VertexDefinitions.h"
#include "GraphicsEnums.h"
#include "GraphicsFeatures.h"
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

	typedef struct UUID BufferHandle;
	typedef struct UUID MeshHandle;
	typedef struct UUID TextureHandle;
	typedef struct UUID RenderTextureHandle;
	typedef struct UUID RenderPassHandle;
	typedef struct UUID ShaderHandle;
	typedef struct UUID PipelineHandle;
	typedef struct UUID DescriptorSetHandle;

	enum BufferType
	{
		BT_TransferRead,
		BT_TransferWrite,
		BT_Vertex,
		BT_Index,
		BT_Storage,
		BT_Uniform,
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

	struct RenderPassInfo
	{
		RenderTextureCreateInfo RenderTextureInfo;
	};

	struct BufferDescriptor
	{
		BufferHandle m_BufferHandle;
		uint32_t m_Offset;
		uint32_t m_Size;
	};

	struct PushConstantsRange
	{
		uint32_t m_Offset = 0;
		uint32_t m_Size = 0;
	};

	struct DescriptorSetInfo
	{
		PushConstantsRange m_PushConstantRange;
		std::vector<BufferDescriptor> m_Buffers;
	};

	class GraphicsDevice
	{
	public:
		GraphicsDevice(Module* pModule);
		virtual ~GraphicsDevice();

		void AddBindingIndex(std::string&& name, uint32_t index);
		uint32_t BindingIndex(const std::string& name) const;

		Debug& Debug();

		virtual bool IsSupported(const APIFeatures& features) const;

	public: /* Rendering commands */
		virtual void Begin() = 0;
		virtual void BeginRenderPass(RenderPassHandle handle) = 0;
		virtual void BeginPipeline(PipelineHandle handle) = 0;
		virtual void End() = 0;
		virtual void EndRenderPass() = 0;
		virtual void EndPipeline() = 0;
		virtual void BindBuffer(BufferHandle buffer) = 0;
		virtual void BindDescriptorSets(PipelineHandle pipeline, std::vector<DescriptorSetHandle> sets) = 0;
		virtual void PushConstants(PipelineHandle pipeline, uint32_t offset, uint32_t size, const void* data) = 0;

		virtual void DrawMesh(MeshHandle handle) = 0;

	public: /* Resource caching */
		PipelineHandle AcquireCachedPipeline(RenderPassHandle renderPass, PipelineData* pPipeline, std::vector<DescriptorSetHandle>&& descriptorSets,
			size_t stride, const std::vector<AttributeType>& attributeTypes);
		MeshHandle AcquireCachedMesh(MeshData* pMesh);

	public: /* Resource mamagement */
		
		/* Buffer */

		/**
		 * @brief Create a buffer on this device
		 * @param name Name of the buffer
		 * @param bufferSize Size of the buffer in bytes
		 * @param type Type of the buffer
		 */
		virtual BufferHandle CreateBuffer(std::string&& name, size_t bufferSize, BufferType type) = 0;

		virtual void AssignBuffer(BufferHandle handle, const void* data) = 0;
		virtual void AssignBuffer(BufferHandle handle, const void* data, uint32_t size) = 0;
		virtual void AssignBuffer(BufferHandle handle, const void* data, uint32_t offset, uint32_t size) = 0;

		/* Mesh */

		/** @brief Create a mesh on this device */
		MeshHandle CreateMesh(MeshData* pMeshData);
		virtual MeshHandle CreateMesh(std::vector<BufferHandle>&& buffers, uint32_t vertexCount,
			uint32_t indexCount, uint32_t stride, PrimitiveType primitiveType,
			const std::vector<AttributeType>& attributeTypes) = 0;

		/* Texture */
		virtual TextureHandle CreateTexture(TextureData* pTexture) = 0;
		virtual TextureHandle CreateTexture(const TextureCreateInfo& textureInfo, const void* pixels = nullptr) = 0;
		/* Render texture */
		virtual RenderTextureHandle CreateRenderTexture(RenderPassHandle renderPass, const RenderTextureCreateInfo& info) = 0;
		/* Render pass */
		virtual RenderPassHandle CreateRenderPass(const RenderPassInfo& info) = 0;
		/* Shader */
		virtual ShaderHandle CreateShader(const FileData* pShaderFileData, const ShaderType& shaderType, const std::string& function) = 0;
		/* Pipeline */
		virtual PipelineHandle CreatePipeline(RenderPassHandle renderPass, PipelineData* pPipeline, std::vector<DescriptorSetHandle>&& descriptorSets,
			size_t stride, const std::vector<AttributeType>& attributeTypes) = 0;

		virtual DescriptorSetHandle CreateDescriptorSet(DescriptorSetInfo&& setInfo) = 0;

		/* Free memory */

		/** @brief Free a buffer from device memory */
		virtual void FreeBuffer(BufferHandle& handle) = 0;
		/** @brief Free a mesh from device memory, this also frees any buffers the mesh owns */
		virtual void FreeMesh(MeshHandle& handle) = 0;
		/** @brief Free a texture from device memory */
		virtual void FreeTexture(MeshHandle& handle) = 0;
		/** @brief Free a render texture from device memory */
		virtual void FreeRenderTexture(RenderTextureHandle& handle) = 0;
		/** @brief Free a render pass from device memory */
		virtual void FreeRenderPass(RenderPassHandle& handle) = 0;
		/** @brief Free a shader from device memory */
		virtual void FreeShader(ShaderHandle& handle) = 0;
		/** @brief Free a pipeline from device memory */
		virtual void FreePipeline(PipelineHandle& handle) = 0;

	protected:
		Module* m_pModule;
		APIFeatures m_APIFeatures;

	private:
		/* Cached handles */
		std::map<UUID, PipelineHandle> m_PipelineHandles;
		std::map<UUID, MeshHandle> m_MeshHandles;

		std::map<std::string, uint32_t> m_BindingIndices;
	};
}
