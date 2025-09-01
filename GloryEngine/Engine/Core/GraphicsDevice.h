#pragma once
#include "VertexDefinitions.h"
#include "GraphicsEnums.h"
#include "GraphicsFeatures.h"
#include "UUID.h"

namespace Glory
{
	enum BufferType
	{
		BT_TransferRead,
		BT_TransferWrite,
		BT_Vertex,
		BT_Index,
		BT_Storage,
		BT_Uniform,
	};

	struct PushConstantsRange
	{
		uint32_t m_Offset = 0;
		uint32_t m_Size = 0;
	};

	struct BufferDescriptorLayout
	{
		BufferType m_Type;
		uint32_t m_BindingIndex;
	};

	struct SamplerDescritporLayout
	{
		uint32_t m_BindingIndex;
	};

	struct DescriptorSetLayoutInfo
	{
		PushConstantsRange m_PushConstantRange;
		std::vector<BufferDescriptorLayout> m_Buffers;
		std::vector<SamplerDescritporLayout> m_Samplers;
		std::vector<std::string> m_SamplerNames;

		bool operator==(const DescriptorSetLayoutInfo& other) const
		{
			return std::memcmp(&m_PushConstantRange, &other.m_PushConstantRange, sizeof(PushConstantsRange)) == 0 &&
				m_Buffers.size() == other.m_Buffers.size() &&
				std::memcmp(m_Buffers.data(), other.m_Buffers.data(), sizeof(BufferDescriptorLayout) * m_Buffers.size()) == 0 &&
				m_Samplers.size() == other.m_Samplers.size() &&
				std::memcmp(m_Samplers.data(), other.m_Samplers.data(), sizeof(SamplerDescritporLayout) * m_Samplers.size()) == 0;
		}
	};
}

namespace std
{
	template <>
	struct hash<Glory::DescriptorSetLayoutInfo>
	{
		size_t operator()(const Glory::DescriptorSetLayoutInfo& info) const noexcept
		{
			size_t hash = 0;
			CombineHash(hash, info.m_PushConstantRange.m_Offset);
			CombineHash(hash, info.m_PushConstantRange.m_Size);

			for (auto& buffer : info.m_Buffers)
			{
				CombineHash(hash, buffer.m_BindingIndex);
				CombineHash(hash, buffer.m_Type);
			}

			for (auto& sampler : info.m_Samplers)
			{
				CombineHash(hash, sampler.m_BindingIndex);
			}
			return hash;
		}
	};
}

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
	typedef struct UUID DescriptorSetLayoutHandle;
	typedef struct UUID DescriptorSetHandle;
	typedef struct UUID CommandBufferHandle;

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

	struct SamplerDescriptor
	{
		TextureHandle m_TextureHandle;
	};

	struct DescriptorSetInfo
	{
		DescriptorSetLayoutHandle m_Layout;
		std::vector<BufferDescriptor> m_Buffers;
		std::vector<SamplerDescriptor> m_Samplers;
	};

	class GraphicsDevice
	{
	public:
		GraphicsDevice(Module* pModule);
		virtual ~GraphicsDevice();

		Debug& Debug();

		virtual bool IsSupported(const APIFeatures& features) const;

	public: /* Rendering commands */
		virtual CommandBufferHandle Begin() = 0;
		virtual void BeginRenderPass(CommandBufferHandle commandBuffer, RenderPassHandle renderPass) = 0;
		virtual void BeginPipeline(CommandBufferHandle commandBuffer, PipelineHandle pipeline) = 0;
		virtual void End(CommandBufferHandle commandBuffer) = 0;
		virtual void EndRenderPass(CommandBufferHandle commandBuffer) = 0;
		virtual void EndPipeline(CommandBufferHandle commandBuffer) = 0;
		virtual void BindDescriptorSets(CommandBufferHandle commandBuffer, PipelineHandle pipeline, std::vector<DescriptorSetHandle> sets, uint32_t firstSet=0) = 0;
		virtual void PushConstants(CommandBufferHandle commandBuffer, PipelineHandle pipeline, uint32_t offset, uint32_t size, const void* data) = 0;

		virtual void DrawMesh(CommandBufferHandle commandBuffer, MeshHandle handle) = 0;
		virtual void Dispatch(CommandBufferHandle commandBuffer, uint32_t x, uint32_t y, uint32_t z) = 0;
		virtual void Commit(CommandBufferHandle commandBuffer) = 0;
		virtual void Wait(CommandBufferHandle commandBuffer) = 0;
		virtual void Release(CommandBufferHandle commandBuffer) = 0;

	public: /* Resource caching */
		PipelineHandle AcquireCachedPipeline(RenderPassHandle renderPass, PipelineData* pPipeline, std::vector<DescriptorSetHandle>&& descriptorSets,
			size_t stride, const std::vector<AttributeType>& attributeTypes);
		MeshHandle AcquireCachedMesh(MeshData* pMesh);
		TextureHandle AcquireCachedTexture(TextureData* pTexture);

	public: /* Resource mamagement */
		
		/* Buffer */

		/**
		 * @brief Create a buffer on this device
		 * @param name Name of the buffer
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

		/* Texture */
		virtual TextureHandle CreateTexture(TextureData* pTexture) = 0;
		virtual TextureHandle CreateTexture(const TextureCreateInfo& textureInfo, const void* pixels=nullptr, size_t dataSize=0) = 0;
		/* Render texture */
		virtual RenderTextureHandle CreateRenderTexture(RenderPassHandle renderPass, const RenderTextureCreateInfo& info) = 0;
		/* Render pass */
		virtual RenderPassHandle CreateRenderPass(const RenderPassInfo& info) = 0;
		/* Shader */
		virtual ShaderHandle CreateShader(const FileData* pShaderFileData, const ShaderType& shaderType, const std::string& function) = 0;
		/* Pipeline */
		virtual PipelineHandle CreatePipeline(RenderPassHandle renderPass, PipelineData* pPipeline, std::vector<DescriptorSetLayoutHandle>&& descriptorSetLayouts,
			size_t stride, const std::vector<AttributeType>& attributeTypes) = 0;
		virtual PipelineHandle CreateComputePipeline(PipelineData* pPipeline, std::vector<DescriptorSetLayoutHandle>&& descriptorSetLayouts) = 0;

		virtual DescriptorSetLayoutHandle CreateDescriptorSetLayout(DescriptorSetLayoutInfo&& setLayoutInfo) = 0;
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
		std::map<UUID, TextureHandle> m_TextureHandles;

		std::map<std::string, uint32_t> m_BindingIndices;
	};
}
