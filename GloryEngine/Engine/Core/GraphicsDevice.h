#pragma once
#include "VertexDefinitions.h"
#include "GraphicsEnums.h"
#include "GraphicsFeatures.h"
#include "GraphicsHandles.h"
#include "UUID.h"

#include <glm/vec4.hpp>

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

	/** @brief Buffer flags */
	enum BufferFlags
	{
		/** @brief None */
		BF_None = 0,
		/** @brief The buffer will be used for reading to the CPU many times */
		BF_Read = 1 << 0,
		/** @brief The buffer will be written to from the CPU many times */
		BF_Write = 1 << 1,
		/** @brief The buffer will both be read and written to from the CPU many times */
		BF_ReadAndWrite = BF_Read | BF_Write,
		/** @brief Force copying to be enabled on this buffer */
		BF_CopyDst = 1 << 2,
	};

	/** @brief Mesh usage */
	enum MeshUsage
	{
		/** @brief The contents of the mesh never change */
		MU_Static = 0,
		/** @brief The contents of the mesh may change every frame */
		MU_Dynamic = 1,
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

	/** @brief Image flags */
	enum ImageFlags
	{
		/** @brief None */
		IF_None = 0,
		/** @brief The buffer will be used for reading to the CPU many times */
		IF_Read = 1 << 0,
		/** @brief The buffer will be written to from the CPU many times */
		IF_Write = 1 << 1,
		/** @brief The buffer will both be read and written to from the CPU many times */
		IF_ReadAndWrite = BF_Read | BF_Write,
		/** @brief Use this image as a copy source */
		IF_CopySrc = 1 << 2,
		/** @brief Use this image as a copy destination */
		IF_CopyDst = 1 << 3,
	};

	struct TextureCreateInfo
	{
		uint32_t m_Width;
		uint32_t m_Height;
		PixelFormat m_PixelFormat;
		PixelFormat m_InternalFormat;
		ImageType m_ImageType;
		DataType m_Type;
		ImageFlags m_Flags;
		ImageAspect m_ImageAspectFlags;
		SamplerSettings m_SamplerSettings = SamplerSettings();
		bool m_SamplingEnabled = true;
	};

	struct Attachment
	{
		Attachment(const std::string& name, const PixelFormat& pixelFormat, const PixelFormat& internalFormat,
			const ImageType& imageType, const ImageAspect& imageAspect, DataType type = DataType::DT_UByte, bool autoBind = true) :
			Name(name), InternalFormat(internalFormat), Format(pixelFormat), ImageType(imageType),
			ImageAspect(imageAspect), m_Type(type), m_AutoBind(autoBind)
		{}
		Attachment(const std::string& name, const TextureCreateInfo& textureInfo, bool autoBind = true) :
			Name(name), InternalFormat(textureInfo.m_InternalFormat), Format(textureInfo.m_PixelFormat), ImageType(textureInfo.m_ImageType),
			ImageAspect(textureInfo.m_ImageAspectFlags), m_Type(textureInfo.m_Type), m_AutoBind(autoBind)
		{}

		//PixelFormat::PF_R8G8B8A8Srgb
		std::string Name;
		PixelFormat InternalFormat;
		PixelFormat Format;
		ImageType ImageType;
		ImageAspect ImageAspect;
		DataType m_Type;
		bool m_AutoBind;
		bool m_SamplingEnabled = true;
		TextureHandle Texture = 0;
	};

	struct RenderTextureCreateInfo
	{
	public:
		RenderTextureCreateInfo() : Width(1), Height(1),
			HasDepth(false), HasStencil(false)
		{}
		RenderTextureCreateInfo(uint32_t width, uint32_t height, bool hasDepth, bool hasStencil = false) :
			Width(width), Height(height), HasDepth(hasDepth), HasStencil(hasStencil)
		{}

		uint32_t Width;
		uint32_t Height;
		bool HasDepth;
		bool HasStencil;
		bool EnableDepthStencilSampling = true;
		std::vector<Attachment> Attachments;
		TextureHandle m_DepthStencilTexture = 0;
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
	class Window;
	class EngineProfiler;

	class Resource;
	class MeshData;
	class ImageData;
	class TextureData;
	class CubemapData;
	class FileData;
	class MaterialData;
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
			m_IDs.clear();
		}

		/**
		 * @brief Add a resource to this container
		 * @param handle Unique resource handle
		 * @param resource Resource to add
		 */
		T& Emplace(GraphicsHandle<T::HandleType> handle, T&& resource)
		{
			m_IDs.push_back(handle.m_ID);
			return m_Resources.emplace(handle.m_ID, std::move(resource)).first->second;
		}

		/** @overload */
		template <class... _Valty>
		T& Emplace(GraphicsHandle<T::HandleType> handle, _Valty&&... _Val)
		{
			m_IDs.push_back(handle.m_ID);
			return m_Resources.emplace(handle.m_ID, forward<_Valty>(_Val)...).first->second;
		}

		/**
		 * @brief Find a resource in this container
		 * @param handle Unique resource handle to find
		 * @returns A pointer to the resource or nullptr if not found
		 */
		T* Find(GraphicsHandle<T::HandleType> handle)
		{
			auto& iter = m_Resources.find(handle.m_ID);
			if (iter == m_Resources.end()) return nullptr;
			return &iter->second;
		}

		/**
		 * @brief Remove a resource from this container
		 * @param @param handle Unique resource handle to remove
		 */
		void Erase(GraphicsHandle<T::HandleType> handle)
		{
			m_Resources.erase(handle.m_ID);
			auto itor = std::find(m_IDs.begin(), m_IDs.end(), handle.m_ID);
			m_IDs.erase(itor);
		}

		/**
		 * @brief Free all resources from this container and clear it
		 * @param @param handler Callback to the free implementation
		 */
		void FreeAll(std::function<void(GraphicsHandle<T::HandleType>)> handler)
		{
			for (auto id : m_IDs)
				handler(GraphicsHandle<T::HandleType>(id));
			Clear();
		}

	private:
		std::unordered_map<UUID, T> m_Resources;
		std::vector<UUID> m_IDs;
	};

	/** @brief Load operation for render passes */
	enum RenderPassLoadOp
	{
		/** @brief Don't care */
		OP_DontCare = 0,
		/** @brief Clear all attachments */
		OP_Clear = 1,
		/** @brief Load all attachments */
		OP_Load = 2,
	};

	/** @brief Position of the renderpass if there are multiple operating on the same framebuffer */
	enum RenderPassPosition
	{
		/** @brief First renderpass */
		RP_Start,
		/** @brief Middle renderpass */
		RP_Middle,
		/** @brief Final renderpass */
		RP_Final
	};

	/** @brief Render pass info */
	struct RenderPassInfo
	{
		RenderTextureHandle RenderTexture = NULL;
		RenderPassLoadOp m_LoadOp = RenderPassLoadOp::OP_Clear;
		RenderPassPosition m_Position = RenderPassPosition::RP_Final;
		RenderTextureCreateInfo RenderTextureInfo;
		glm::vec4 m_ClearColor{ 0.0f, 0.0f, 0.0f, 0.0f };
		float m_DepthClear{ 1.0f };
		uint8_t m_StencilClear{ 0 };
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

	/** @brief Buffer descriptor update info */
	struct BufferDescriptorUpdate
	{
		BufferHandle m_BufferHandle;
		uint32_t m_Offset;
		uint32_t m_Size;
		uint32_t m_DescriptorIndex;
	};

	/** @brief Sampler descriptor update info */
	struct SamplerDescriptorUpdate
	{
		TextureHandle m_TextureHandle;
		uint32_t m_DescriptorIndex;
	};

	/** @brief Descriptor set update info */
	struct DescriptorSetUpdateInfo
	{
		std::vector<BufferDescriptorUpdate> m_Buffers;
		std::vector<SamplerDescriptorUpdate> m_Samplers;
	};

	enum AccessFlags : uint32_t
	{
		AF_None = 0,
		AF_IndirectCommandRead = 1 << 0,
		AF_IndexRead = 1 << 1,
		AF_VertexAttributeRead = 1 << 2,
		AF_UniformRead = 1 << 3,
		AF_InputAttachmentRead = 1 << 4,
		AF_ShaderRead = 1 << 5,
		AF_ShaderWrite = 1 << 6,
		AF_ColorAttachmentRead = 1 << 7,
		AF_ColorAttachmentWrite = 1 << 8,
		AF_DepthStencilAttachmentRead = 1 << 9,
		AF_DepthStencilAttachmentWrite = 1 << 10,
		AF_CopySrc = 1 << 11,
		AF_CopyDst = 1 << 12,
		AF_CPURead = 1 << 13,
		AF_CPUWrite = 1 << 14,
		AF_MemoryRead = 1 << 15,
		AF_MemoryWrite = 1 << 16,
	};

	struct BufferBarrier
	{
		BufferHandle m_Buffer;
		AccessFlags m_SrcAccessMask;
		AccessFlags m_DstAccessMask;

		size_t m_Offset = 0;
		size_t m_Size = 0;
	};

	struct ImageBarrier
	{
		TextureHandle m_Texture;
		AccessFlags m_SrcAccessMask;
		AccessFlags m_DstAccessMask;
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
		/** @brief Helper for getting the profiler */
		EngineProfiler& Profiler();

		/**
		 * @brief Check whether certain API features are supported
		 * @param features Features to check for
		 */
		virtual bool IsSupported(const APIFeatures& features) const;
		virtual ViewportOrigin GetViewportOrigin() const { return ViewportOrigin::BottomLeft; }

		void Initialize();

		TextureHandle GetDefaultTexture() const { return m_DefaultTexture; }

	public: /* Rendering commands */
		/** @brief Create a new command buffer */
		virtual CommandBufferHandle CreateCommandBuffer() = 0;
		/**
		 * @brief Create a new command buffer and call Begin() on it
		 * @returns The handle to the command buffer
		 */
		virtual CommandBufferHandle Begin();
		/**
		 * @brief Begin recording on a command buffer
		 * @param commandBuffer The handle to the command buffer
		 */
		virtual void Begin(CommandBufferHandle commandBuffer) = 0;
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
		virtual void BindDescriptorSets(CommandBufferHandle commandBuffer, PipelineHandle pipeline, const std::vector<DescriptorSetHandle>& sets, uint32_t firstSet=0) = 0;
		/**
		 * @brief Push push constants onto a command buffer
		 * @param commandBuffer The handle to the command buffer
		 * @param pipeline The handle to the pipeline to push the constants to
		 * @param offset The destination offset of the push constants
		 * @param size Size of the constants to push
		 * @param data The push constants data
		 */
		virtual void PushConstants(CommandBufferHandle commandBuffer, PipelineHandle pipeline, uint32_t offset, uint32_t size, const void* data, ShaderTypeFlag shaderStages) = 0;

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
		virtual void Commit(CommandBufferHandle commandBuffer, const std::vector<SemaphoreHandle>& waitSemaphores={},
			const std::vector<SemaphoreHandle>& signalSemaphore={}) = 0;

		enum WaitResult
		{
			WR_Success,
			WR_Timeout,
			WR_Fail
		};

		/**
		 * @brief Wait for a command buffer to finish on the GPU
		 * @param commandBuffer The handle to the command buffer
		 */
		virtual WaitResult Wait(CommandBufferHandle commandBuffer, uint64_t timeout=UINT64_MAX) = 0;
		/**
		 * @brief Reset and release a command buffer
		 * @param commandBuffer The handle to the command buffer
		 */
		virtual void Release(CommandBufferHandle commandBuffer) = 0;
		/**
		 * @brief Reset a command buffer
		 * @param commandBuffer The handle to the command buffer
		 */
		virtual void Reset(CommandBufferHandle commandBuffer) = 0;

		/**
		 * @brief Record a set viewport command
		 * @param commandBuffer The handle to the command buffer
		 * @param x Viewport X position
		 * @param y Viewport Y position
		 * @param width Viewport width
		 * @param height Viewport height
		 * @param minDepth Viewport minimum depth
		 * @param maxDepth Viewport maximum depth
		 */
		virtual void SetViewport(CommandBufferHandle commandBuffer, float x, float y, float width, float height, float minDepth=0.0f, float maxDepth=1.0f) = 0;
		/**
		 * @brief Record a set scissor command
		 * @param commandBuffer The handle to the command buffer
		 * @param x Scissor X position
		 * @param y Scissor Y position
		 * @param width Scissor width
		 * @param height Scissor height
		 */
		virtual void SetScissor(CommandBufferHandle commandBuffer, int x, int y, uint32_t width, uint32_t height) = 0;

		/**
		 * @brief Record commands to draw a basic quad, this quad consists of 6 vec3's for positions
		 * @param commandBuffer The handle to the command buffer
		 */
		void DrawQuad(CommandBufferHandle commandBuffer);
		/**
		 * @brief Record commands to draw a basic unit cube, this cube consists of 36 vec3's for positions
		 * @param commandBuffer The handle to the command buffer
		 */
		void DrawUnitCube(CommandBufferHandle commandBuffer);

		/**
		 * @brief Push a pipeline barrier onto a command buffer
		 * @param commandBuffer The handle to the command buffer
		 * @param buffers Buffers for buffer memory barriers
		 * @param textures Textures for image memory barriers
		 * @param srcStage Source stage
		 * @param dstStage Destination stage
		 */
		virtual void PipelineBarrier(CommandBufferHandle commandBuffer, const std::vector<BufferBarrier>& buffers,
			const std::vector<ImageBarrier>& images, PipelineStageFlagBits srcStage, PipelineStageFlagBits dstStage) = 0;

		virtual void CopyImage(CommandBufferHandle commandBuffer, TextureHandle src, TextureHandle dst) = 0;

		enum SwapchainResult
		{
			S_Error = -1,
			S_Success,
			S_OutOfDate,
		};

		/**
		 * @brief Aqcuire the next swapchain image for rendering
		 * @param swapchain The swapchain to get the image from
		 * @param imageIndex The swapchain image index
		 * @param signalSemaphore Semaphore to signal when the image is ready
		 * @returns Result of aqcuiring the image, @ref SwapchainResult::S_Success on success,
		 *			@ref SwapchainResult::S_OutOfDate if the swapchain needs to be recreated.
		 */
		virtual SwapchainResult AcquireNextSwapchainImage(SwapchainHandle swapchain, uint32_t* imageIndex,
			SemaphoreHandle signalSemaphore=NULL) = 0;
		/**
		 * @brief Swap the backbuffer
		 * @param swapchain The swapchain to present from
		 * @param imageIndex The swapchain image index to present
		 * @param waitSemaphores Semaphores to wait on
		 * @returns Result of presenting the image, @ref SwapchainResult::S_Success on success,
		 *			@ref SwapchainResult::S_OutOfDate if the swapchain needs to be recreated.
		 */
		virtual SwapchainResult Present(SwapchainHandle swapchain, uint32_t imageIndex, const std::vector<SemaphoreHandle>& waitSemaphores={}) = 0;

		virtual void WaitIdle() = 0;

	public: /* Resource caching */
		/**
		 * @brief Acquire a cached pipeline or create a new one
		 * @param renderPass The handle to the render pass
		 * @param pPipeline Pipeline data to create a new pipeline from
		 * @param descriptorSets The descriptor sets to use in this pipeline
		 * @param stride Size of a vertex
		 * @param attributeTypes Attribute types
		 *
		 * The pipeline gets recreated if the shaders have changed,
		 * and gets updated if its settings were changed.
		 */
		PipelineHandle AcquireCachedPipeline(RenderPassHandle renderPass, PipelineData* pPipeline,
			std::vector<DescriptorSetLayoutHandle>&& descriptorSets, size_t stride,
			const std::vector<AttributeType>& attributeTypes);
		/**
		 * @brief Acquire a cached mesh or create a new one
		 * @param pMesh The mesh data to create a mesh from
		 */
		MeshHandle AcquireCachedMesh(MeshData* pMesh, MeshUsage usage=MeshUsage::MU_Static);
		/**
		 * @brief Acquire a cached texture or create a new one
		 * @param pTexture The texture data to create a texture from
		 */
		TextureHandle AcquireCachedTexture(TextureData* pTexture);
		/**
		 * @brief Acquire a cached cubemap texture or create a new one
		 * @param pTexture The cubemap data to create a texture from
		 */
		TextureHandle AcquireCachedTexture(CubemapData* pCubemap);
		/**
		 * @brief Check if a texture exists on this device
		 * @param pTexture The texture data to check for
		 */
		bool CachedTextureExists(TextureData* pTexture);

		/**
		 * @brief Acquire a cached shader or create a new one
		 * @param pShaderFileData Shader data
		 * @param shaderType Type of the shader
		 * @param function Main function to invoke in the shader
		 */
		ShaderHandle AcquireCachedShader(const FileData* pShaderFileData, const ShaderType& shaderType, const std::string& function);

	public: /* Resource management */
		
		/* Buffer */

		/**
		 * @brief Create a buffer on this device
		 * @param name Name of the buffer
		 * @param bufferSize Size of the buffer in bytes
		 * @param type Type of the buffer
		 */
		virtual BufferHandle CreateBuffer(size_t bufferSize, BufferType type, BufferFlags flags) = 0;
		/**
		 * @brief Resize a buffer on this device
		 * @param buffer Buffer to resize
		 * @param bufferSize New size of the buffer in bytes
		 */
		virtual void ResizeBuffer(BufferHandle buffer, size_t bufferSize) = 0;
		/**
		 * @brief Get the current size of a buffer on this device
		 * @param buffer Buffer
		 * @returns Size of the buffer or 0 if invalid
		 */
		virtual size_t BufferSize(BufferHandle buffer) = 0;

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
		/**
		 * @brief Copy data from a buffer to memory
		 * @param handle The handle to the buffer
		 * @param outData Memory to copy to
		 * @param offset Offset into the source buffer
		 * @param size Size of the data to copy
		 */
		virtual void ReadBuffer(BufferHandle handle, void* outData, uint32_t offset, uint32_t size) = 0;

		/* Mesh */

		/**
		 * @brief Create a mesh on this device
		 * @param pMeshData Mesh data
		 */
		MeshHandle CreateMesh(MeshData* pMeshData, MeshUsage usage=MeshUsage::MU_Static);
		/**
		 * @brief Create a mesh on this device
		 * @param buffers Vertex buffers and an index buffer at the last index
		 * @param vertexCount Numnber of vertices
		 * @param indexCount Numnber of indexCount
		 * @param stride Size of a vertex
		 * @param attributeTypes Attribute types of the vertices in the mesh
		 */
		virtual MeshHandle CreateMesh(std::vector<BufferHandle>&& buffers, uint32_t vertexCount,
			uint32_t indexCount, uint32_t stride, const std::vector<AttributeType>& attributeTypes) = 0;

		/**
		 * @brief Update a mesh
		 * @param mesh Mesh to update
		 * @param buffers New buffers to bind to the mesh, leave empty to keep original
		 * @param vertexCount New number of vertices
		 * @param indexCount New number of indices
		 */
		virtual void UpdateMesh(MeshHandle mesh, std::vector<BufferHandle>&& buffers,
			uint32_t vertexCount, uint32_t indexCount) = 0;

		/** @overload */
		virtual void UpdateMesh(MeshHandle mesh, MeshData* pMeshData) = 0;

		/* Texture */

		/**
		 * @brief Create a texture on this device
		 * @param pTexture Texture data
		 */
		virtual TextureHandle CreateTexture(TextureData* pTexture) = 0;
		/**
		 * @brief Create a cubemap texture on this device
		 * @param pCubemap Cubemap data
		 */
		virtual TextureHandle CreateTexture(CubemapData* pCubemap) = 0;
		/**
		 * @brief Create a texture on this device
		 * @param textureInfo Texture creation info
		 * @param pixels Pixel data to copy to the texture
		 * @param dataSize Total size of the pixel data
		 */
		virtual TextureHandle CreateTexture(const TextureCreateInfo& textureInfo, const void* pixels=nullptr, size_t dataSize=0) = 0;

		virtual void UpdateTexture(TextureHandle texture, TextureData* pTextureData) = 0;
		virtual void ReadTexturePixels(TextureHandle texture, void* dst, size_t offset, size_t size) = 0;

		/* Render texture */

		/**
		 * @brief Create a render texture on this device
		 * @param renderPass The render pass it belongs to
		 * @param info Render texture creation info
		 */
		virtual RenderTextureHandle CreateRenderTexture(RenderPassHandle renderPass, RenderTextureCreateInfo&& info) = 0;

		/**
		 * @brief Get an attachment texture of a render texture
		 * @param renderTexture Render texture handle to get the attachment from
		 * @param index Index of the attachment to get
		 */
		virtual TextureHandle GetRenderTextureAttachment(RenderTextureHandle renderTexture, size_t index) = 0;

		/**
		 * @brief Resize a render texture
		 * @param renderTexture Render texture handle to resize
		 * @param width New width of the render texture
		 * @param height New height of the render texture
		 */
		virtual void ResizeRenderTexture(RenderTextureHandle renderTexture, uint32_t width, uint32_t height) = 0;

		/* Render pass */
		
		/**
		 * @brief Create a render pass on this device
		 * @param info Render pass creation info
		 */
		virtual RenderPassHandle CreateRenderPass(RenderPassInfo&& info) = 0;

		/**
		 * @brief Get the render texture of a render pass
		 * @param renderPass Render pass to get the render texture from
		 */
		virtual RenderTextureHandle GetRenderPassRenderTexture(RenderPassHandle renderPass) = 0;

		/**
		 * @brief Set the clear color depth and stencil value for a render pass
		 * @param renderPass Render pass to update
		 * @param color Clear color
		 * @param depth Depth value
		 * @param stencil Stencil value
		 */
		virtual void SetRenderPassClear(RenderPassHandle renderPass, const glm::vec4& color, float depth=1.0f, uint8_t stencil=0) = 0;

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
		virtual PipelineHandle CreatePipeline(RenderPassHandle renderPass, PipelineData* pPipeline,
			std::vector<DescriptorSetLayoutHandle>&& descriptorSetLayouts, size_t stride,
			const std::vector<AttributeType>& attributeTypes) = 0;

		/**
		 * @brief Update a graphics pipelines settings
		 * @param pipeline Pipeline to update
		 * @param pPipeline Pipeline data
		 */
		virtual void UpdatePipelineSettings(PipelineHandle pipeline, PipelineData* pPipeline) = 0;
		/**
		 * @brief Recreate a graphics pipeline on this device
		 * @param pipeline Pipeline to recreate
		 * @param pPipeline Pipeline data
		 * @param descriptorSetLayouts Descriptor set layouts that this pipeline will use
		 * @param stride Size of the vertex type used by this pipeline
		 * @param attributeTypes Attribute types of the vertex type used by this pipeline
		 */
		virtual void RecreatePipeline(PipelineHandle pipeline, PipelineData* pPipeline) = 0;

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
		/**
		 * @brief Update a descriptor set
		 * @param descriptorSet Descriptor set to update
		 * @param setWriteInfo @ref DescriptorSetUpdateInfo struct with info on which descriptors in the set to update
		 */
		virtual void UpdateDescriptorSet(DescriptorSetHandle descriptorSet, const DescriptorSetUpdateInfo& setWriteInfo) = 0;

		/* Swap chain */
		virtual SwapchainHandle CreateSwapchain(Window* pWindow, bool vsync=false, uint32_t minImageCount=0) = 0;
		virtual uint32_t GetSwapchainImageCount(SwapchainHandle swapchain) = 0;
		virtual TextureHandle GetSwapchainImage(SwapchainHandle swapchain, uint32_t imageIndex) = 0;
		virtual void RecreateSwapchain(SwapchainHandle swapchain) = 0;

		/* Synchronization */
		virtual SemaphoreHandle CreateSemaphore() = 0;

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
		/** @brief Free a swap chain from device memory */
		virtual void FreeSwapchain(SwapchainHandle& handle) = 0;
		/** @brief Free a semaphore from device memory */
		virtual void FreeSemaphore(SemaphoreHandle& handle) = 0;

	protected:
		virtual void OnInitialize() {}

	protected:
		Module* m_pModule;
		APIFeatures m_APIFeatures;
		MeshHandle m_ScreenMesh;
		MeshHandle m_UnitCubeMesh;
		TextureHandle m_DefaultTexture;

	private:
		/* Cached handles */
		std::map<UUID, PipelineHandle> m_PipelineHandles;
		std::map<UUID, MeshHandle> m_MeshHandles;
		std::map<UUID, TextureHandle> m_TextureHandles;
		std::map<size_t, ShaderHandle> m_ShaderHandles;
	};
}
