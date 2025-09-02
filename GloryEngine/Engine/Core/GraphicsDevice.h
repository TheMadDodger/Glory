#pragma once
#include "VertexDefinitions.h"
#include "GraphicsEnums.h"
#include "GraphicsFeatures.h"
#include "GraphicsHandles.h"
#include "UUID.h"

namespace Glory
{
	/** @brief Buffer type */
	enum BufferType
	{
		BT_TransferRead,
		BT_TransferWrite,
		BT_Vertex,
		BT_Index,
		BT_Storage,
		BT_Uniform,
	};

	/** @brief Push constants range */
	struct PushConstantsRange
	{
		uint32_t m_Offset = 0;
		uint32_t m_Size = 0;
		ShaderTypeFlag m_ShaderStages = ShaderTypeFlag::STF_All;
	};

	/** @brief Buffer descriptor layout */
	struct BufferDescriptorLayout
	{
		BufferType m_Type;
		uint32_t m_BindingIndex;
		ShaderTypeFlag m_ShaderStages = ShaderTypeFlag::STF_All;
	};

	/** @brief Sampler descriptor layout */
	struct SamplerDescritporLayout
	{
		uint32_t m_BindingIndex;
		ShaderTypeFlag m_ShaderStages = ShaderTypeFlag::STF_All;
	};

	/** @brief Descriptor set layout info */
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
				std::memcmp(m_Buffers.data(), other.m_Buffers.data(), sizeof(BufferDescriptorLayout)*m_Buffers.size()) == 0 &&
				m_Samplers.size() == other.m_Samplers.size() &&
				std::memcmp(m_Samplers.data(), other.m_Samplers.data(), sizeof(SamplerDescritporLayout)*m_Samplers.size()) == 0;
		}
	};
}

namespace std
{
	/** @brief Hash generator for DescriptorSetLayoutInfo */
	template <>
	struct hash<Glory::DescriptorSetLayoutInfo>
	{
		/** @brief Hash function */
		size_t operator()(const Glory::DescriptorSetLayoutInfo& info) const noexcept
		{
			size_t hash = 0;
			CombineHash(hash, info.m_PushConstantRange.m_Offset);
			CombineHash(hash, info.m_PushConstantRange.m_Size);
			CombineHash(hash, info.m_PushConstantRange.m_ShaderStages);

			for (auto& buffer : info.m_Buffers)
			{
				CombineHash(hash, buffer.m_BindingIndex);
				CombineHash(hash, buffer.m_Type);
				CombineHash(hash, buffer.m_ShaderStages);
			}

			for (auto& sampler : info.m_Samplers)
			{
				CombineHash(hash, sampler.m_BindingIndex);
				CombineHash(hash, sampler.m_ShaderStages);
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

	/**
	 * @brief Container for graphics resources
	 * @param T Type of the resource
	 */
	template<class T>
	struct GraphicsResources
	{
	public:
		/** @brief Clear this container */
		void Clear()
		{
			m_Resources.clear();
		}

		/**
		 * @brief Add a resource to this container
		 * @param id ID of the resource
		 * @param resource Resource to add
		 */
		T& Emplace(UUID id, T&& resource)
		{
			return m_Resources.emplace(id, std::move(resource)).first->second;
		}

		/** @overload */
		template <class... _Valty>
		T& Emplace(UUID id, _Valty&&... _Val)
		{
			return m_Resources.emplace(id, forward<_Valty>(_Val)...).first->second;
		}

		/**
		 * @brief Find a resource in this container
		 * @param id ID of the resource to find
		 * @returns A pointer to the resource or nullptr if not found
		 */
		T* Find(UUID id)
		{
			auto& iter = m_Resources.find(id);
			if (iter == m_Resources.end()) return nullptr;
			return &iter->second;
		}

		/**
		 * @brief Remove a resource from this container
		 * @param id ID of the resource to remove
		 */
		void Erase(UUID id)
		{
			m_Resources.erase(id);
		}

	private:
		std::unordered_map<UUID, T> m_Resources;
	};

	/** @brief Render pass info */
	struct RenderPassInfo
	{
		RenderTextureCreateInfo RenderTextureInfo;
	};

	/** @brief Buffer descriptor info */
	struct BufferDescriptor
	{
		BufferHandle m_BufferHandle;
		uint32_t m_Offset;
		uint32_t m_Size;
	};

	/** @brief Sampler descriptor info */
	struct SamplerDescriptor
	{
		TextureHandle m_TextureHandle;
	};

	/** @brief Descriptor set info */
	struct DescriptorSetInfo
	{
		DescriptorSetLayoutHandle m_Layout;
		std::vector<BufferDescriptor> m_Buffers;
		std::vector<SamplerDescriptor> m_Samplers;
	};

	/** @brief Graphics device abstraction */
	class GraphicsDevice
	{
	public:
		/**
		 * @brief Constructor
		 * @param pModule Module that owns the device
		 */
		GraphicsDevice(Module* pModule);
		/** @brief Destructor */
		virtual ~GraphicsDevice();

		/** @brief Helper for getting the debug logger */
		Debug& Debug();

		/**
		 * @brief Check whether certain API features are supported
		 * @param features Features to check for
		 */
		virtual bool IsSupported(const APIFeatures& features) const;

	public: /* Rendering commands */
		/**
		 * @brief Begin recording a new command buffer
		 * @returns The handle to the command buffer
		 */
		virtual CommandBufferHandle Begin() = 0;
		/**
		 * @brief Push a begin render pass onto a command buffer
		 * @param commandBuffer The handle to the command buffer
		 * @param renderPass The handle to the render pass
		 */
		virtual void BeginRenderPass(CommandBufferHandle commandBuffer, RenderPassHandle renderPass) = 0;
		/**
		 * @brief Push a begin/bind pipeline onto a command buffer
		 * @param commandBuffer The handle to the command buffer
		 * @param pipeline The handle to the pipeline
		 */
		virtual void BeginPipeline(CommandBufferHandle commandBuffer, PipelineHandle pipeline) = 0;
		/**
		 * @brief End recording of a command buffer
		 * @param commandBuffer The handle to the command buffer
		 */
		virtual void End(CommandBufferHandle commandBuffer) = 0;
		/**
		 * @brief Push an end render pass onto a command buffer
		 * @param commandBuffer The handle to the command buffer
		 */
		virtual void EndRenderPass(CommandBufferHandle commandBuffer) = 0;
		/**
		 * @brief Push an end/unbind pipeline onto a command buffer
		 * @param commandBuffer The handle to the command buffer
		 */
		virtual void EndPipeline(CommandBufferHandle commandBuffer) = 0;
		/**
		 * @brief Push a descriptor set bind onto a command buffer
		 * @param commandBuffer The handle to the command buffer
		 * @param pipeline The handle to the pipeline binding the descriptor set to
		 * @param sets Handles of descriptor sets to bind
		 * @param firstSet The index to the first set to bind to
		 */
		virtual void BindDescriptorSets(CommandBufferHandle commandBuffer, PipelineHandle pipeline, std::vector<DescriptorSetHandle> sets, uint32_t firstSet=0) = 0;
		/**
		 * @brief Push push constants onto a command buffer
		 * @param commandBuffer The handle to the command buffer
		 * @param pipeline The handle to the pipeline to push the constants to
		 * @param offset The destination offset of the push constants
		 * @param size Size of the constants to push
		 * @param data The push constants data
		 */
		virtual void PushConstants(CommandBufferHandle commandBuffer, PipelineHandle pipeline, uint32_t offset, uint32_t size, const void* data) = 0;

		/**
		 * @brief Push a draw mesh onto the command buffer
		 * @param commandBuffer The handle to the command buffer
		 * @param handle Mesh to draw
		 */
		virtual void DrawMesh(CommandBufferHandle commandBuffer, MeshHandle handle) = 0;
		/**
		 * @brief Push a dispatch onto the command buffer
		 * @param commandBuffer The handle to the command buffer
		 * @param x Number of X workgroups
		 * @param y Number of Y workgroups
		 * @param z Number of Z workgroups
		 */
		virtual void Dispatch(CommandBufferHandle commandBuffer, uint32_t x, uint32_t y, uint32_t z) = 0;
		/**
		 * @brief Commit a command buffer to the GPU
		 * @param commandBuffer The handle to the command buffer
		 */
		virtual void Commit(CommandBufferHandle commandBuffer) = 0;
		/**
		 * @brief Wait for a command buffer to finish on the GPU
		 * @param commandBuffer The handle to the command buffer
		 */
		virtual void Wait(CommandBufferHandle commandBuffer) = 0;
		/**
		 * @brief Reset and release a command buffer
		 * @param commandBuffer The handle to the command buffer
		 */
		virtual void Release(CommandBufferHandle commandBuffer) = 0;

	public: /* Resource caching */
		/**
		 * @brief Acquire a cached pipeline or create a new one
		 * @param renderPass The handle to the render pass
		 * @param pPipeline Pipeline data to create a new pipeline from
		 * @param descriptorSets The descriptor sets to use in this pipeline
		 * @param stride Size of a vertex
		 * @param attributeTypes Attribute types
		 */
		PipelineHandle AcquireCachedPipeline(RenderPassHandle renderPass, PipelineData* pPipeline,
			std::vector<DescriptorSetLayoutHandle>&& descriptorSets, size_t stride, const std::vector<AttributeType>& attributeTypes);
		/**
		 * @brief Acquire a cached mesh or create a new one
		 * @param pMesh The mesh data to create a mesh from
		 */
		MeshHandle AcquireCachedMesh(MeshData* pMesh);
		/**
		 * @brief Acquire a cached texture or create a new one
		 * @param pTexture The texture data to create a texture from
		 */
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

		/** @overload */
		virtual void AssignBuffer(BufferHandle handle, const void* data) = 0;
		/** @overload */
		virtual void AssignBuffer(BufferHandle handle, const void* data, uint32_t size) = 0;
		/**
		 * @brief Assign data to a buffer
		 * @param handle The handle to the buffer
		 * @param data The data to assign
		 * @param offset Offset into the destination buffer
		 * @param size Size of the data to assign
		 */
		virtual void AssignBuffer(BufferHandle handle, const void* data, uint32_t offset, uint32_t size) = 0;

		/* Mesh */

		/**
		 * @brief Create a mesh on this device
		 * @param pMeshData Mesh data
		 */
		MeshHandle CreateMesh(MeshData* pMeshData);
		/**
		 * @brief Create a mesh on this device
		 * @param buffers Vertex buffers and an index buffer at the last index
		 * @param vertexCount Numnber of vertices
		 * @param indexCount Numnber of indexCount
		 * @param stride Size of a vertex
		 * @param primitiveType Type of the primitives in the mesh
		 * @param attributeTypes Attribute types of the vertices in the mesh
		 */
		virtual MeshHandle CreateMesh(std::vector<BufferHandle>&& buffers, uint32_t vertexCount,
			uint32_t indexCount, uint32_t stride, PrimitiveType primitiveType,
			const std::vector<AttributeType>& attributeTypes) = 0;

		/* Texture */

		/**
		 * @brief Create a texture on this device
		 * @param pTexture Texture data
		 */
		virtual TextureHandle CreateTexture(TextureData* pTexture) = 0;
		/**
		 * @brief Create a texture on this device
		 * @param textureInfo Texture creation info
		 * @param pixels Pixel data to copy to the texture
		 * @param dataSize Total size of the pixel data
		 */
		virtual TextureHandle CreateTexture(const TextureCreateInfo& textureInfo, const void* pixels=nullptr, size_t dataSize=0) = 0;

		/* Render texture */

		/**
		 * @brief Create a render texture on this device
		 * @param renderPass The render pass it belongs to
		 * @param info Render texture creation info
		 */
		virtual RenderTextureHandle CreateRenderTexture(RenderPassHandle renderPass, const RenderTextureCreateInfo& info) = 0;

		/* Render pass */
		
		/**
		 * @brief Create a render pass on this device
		 * @param info Render pass creation info
		 */
		virtual RenderPassHandle CreateRenderPass(const RenderPassInfo& info) = 0;

		/* Shader */

		/**
		 * @brief Create a shader on this device
		 * @param pShaderFileData Shader file
		 * @param shaderType Shader type
		 * @param function Main function of the shader
		 */
		virtual ShaderHandle CreateShader(const FileData* pShaderFileData, const ShaderType& shaderType, const std::string& function) = 0;

		/* Pipeline */
		
		/**
		 * @brief Create a graphics pipeline on this device
		 * @param renderPass Render pass to create the pipeline for
		 * @param pPipeline Pipeline data
		 * @param descriptorSetLayouts Descriptor set layouts that this pipeline will use
		 * @param stride Size of the vertex type used by this pipeline
		 * @param attributeTypes Attribute types of the vertex type used by this pipeline
		 */
		virtual PipelineHandle CreatePipeline(RenderPassHandle renderPass, PipelineData* pPipeline, std::vector<DescriptorSetLayoutHandle>&& descriptorSetLayouts,
			size_t stride, const std::vector<AttributeType>& attributeTypes) = 0;

		/**
		 * @brief Create a compute pipeline on this device
		 * @param pPipeline Pipeline data
		 * @param descriptorSetLayouts Descriptor set layouts that this pipeline will use
		 */
		virtual PipelineHandle CreateComputePipeline(PipelineData* pPipeline, std::vector<DescriptorSetLayoutHandle>&& descriptorSetLayouts) = 0;

		/* Descriptor sets */

		/**
		 * @brief Create a descriptor set layout
		 * @param setLayoutInfo Descriptor set layout info
		 */
		virtual DescriptorSetLayoutHandle CreateDescriptorSetLayout(DescriptorSetLayoutInfo&& setLayoutInfo) = 0;
		/**
		 * @brief Create a descriptor set
		 * @param setInfo Descriptor set info
		 */
		virtual DescriptorSetHandle CreateDescriptorSet(DescriptorSetInfo&& setInfo) = 0;

		/* Free memory */

		/** @brief Free a buffer from device memory */
		virtual void FreeBuffer(BufferHandle& handle) = 0;
		/** @brief Free a mesh from device memory, this also frees any buffers the mesh owns */
		virtual void FreeMesh(MeshHandle& handle) = 0;
		/** @brief Free a texture from device memory */
		virtual void FreeTexture(TextureHandle& handle) = 0;
		/** @brief Free a render texture from device memory */
		virtual void FreeRenderTexture(RenderTextureHandle& handle) = 0;
		/** @brief Free a render pass from device memory */
		virtual void FreeRenderPass(RenderPassHandle& handle) = 0;
		/** @brief Free a shader from device memory */
		virtual void FreeShader(ShaderHandle& handle) = 0;
		/** @brief Free a pipeline from device memory */
		virtual void FreePipeline(PipelineHandle& handle) = 0;
		/** @brief Free a descriptor set layout from device memory */
		virtual void FreeDescriptorSetLayout(DescriptorSetLayoutHandle& handle) = 0;
		/** @brief Free a descriptor set from device memory */
		virtual void FreeDescriptorSet(DescriptorSetHandle& handle) = 0;

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
