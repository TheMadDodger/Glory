#pragma once
#include <ostream>

#include "UUID.h"

namespace Glory
{
	/** @brief Graphics handle types */
	enum GraphicsHandleType
	{
		/** @brief Buffer */
		H_Buffer,
		/** @brief Mesh */
		H_Mesh,
		/** @brief Texture */
		H_Texture,
		/** @brief RenderTexture */
		H_RenderTexture,
		/** @brief RenderPass */
		H_RenderPass,
		/** @brief Shader */
		H_Shader,
		/** @brief Pipeline */
		H_Pipeline,
		/** @brief DescriptorSetLayout */
		H_DescriptorSetLayout,
		/** @brief DescriptorSet */
		H_DescriptorSet,
		/** @brief CommandBuffer */
		H_CommandBuffer,
		/** @brief Swapchain */
		H_Swapchain,
		/** @brief Semaphore */
		H_Semaphore,
		/** @brief Fence */
		H_Fence,
	};

	/** @brief Base struct for graphics resource handles */
	template<GraphicsHandleType T>
	struct GraphicsHandle
	{
		/** @brief Constructor */
		GraphicsHandle() : m_ID(UUID()) {}
		/** @overload */
		GraphicsHandle(std::nullptr_t) : m_ID(0ull) {}
		/** @overload */
		GraphicsHandle(UUID id) : m_ID(id) {}
		/** @overload */
		GraphicsHandle(const GraphicsHandle& other) : m_ID(other.m_ID) {}
		/** @overload */
		GraphicsHandle(GraphicsHandle&& other) : m_ID(other.m_ID) {}
		/** @brief Destructor */
		~GraphicsHandle() { m_ID = 0; }
		/** @brief Convert to UUID */
		operator UUID() const { return m_ID; }
		/** @brief Convert to bool */
		operator bool() const { return m_ID != NULL; }
		/** @brief Set to null */
		GraphicsHandle& operator=(std::nullptr_t) { m_ID = NULL; return *this; }
		/** @overload Copy assignment */
		GraphicsHandle& operator=(const GraphicsHandle& other) { m_ID = other.m_ID; return *this; }
		/** @overload Move assignment */
		GraphicsHandle& operator=(GraphicsHandle&& other) { m_ID = other.m_ID; return *this; }
		/** @overload Comparator */
		bool operator==(const GraphicsHandle& other) const { return m_ID == other.m_ID; }
		/** @overload Comparator */
		bool operator==(std::nullptr_t) const { return m_ID == NULL; }
		/** @overload Comparator */
		bool operator==(UUID id) const { return m_ID == id; }

		UUID m_ID;
	};

	/** @brief Stream operator overload for graphics handles */
	template<GraphicsHandleType T>
	std::ostream& operator<<(std::ostream& os, const GraphicsHandle<T>& handle)
	{
		os << handle.m_ID;
		return os;
	}

	/** @brief Buffer handle */
	typedef GraphicsHandle<H_Buffer> BufferHandle;
	/** @brief Mesh handle */
	typedef GraphicsHandle<H_Mesh> MeshHandle;
	/** @brief Texture handle */
	typedef GraphicsHandle<H_Texture> TextureHandle;
	/** @brief RenderTexture handle */
	typedef GraphicsHandle<H_RenderTexture> RenderTextureHandle;
	/** @brief RenderPass handle */
	typedef GraphicsHandle<H_RenderPass> RenderPassHandle;
	/** @brief Shader handle */
	typedef GraphicsHandle<H_Shader> ShaderHandle;
	/** @brief Pipeline handle */
	typedef GraphicsHandle<H_Pipeline> PipelineHandle;
	/** @brief DescriptorSetLayout handle */
	typedef GraphicsHandle<H_DescriptorSetLayout> DescriptorSetLayoutHandle;
	/** @brief DescriptorSet handle */
	typedef GraphicsHandle<H_DescriptorSet> DescriptorSetHandle;
	/** @brief CommandBuffer handle */
	typedef GraphicsHandle<H_CommandBuffer> CommandBufferHandle;
	/** @brief Swapchain handle */
	typedef GraphicsHandle<H_Swapchain> SwapchainHandle;
	/** @brief Semaphore handle */
	typedef GraphicsHandle<H_Semaphore> SemaphoreHandle;
	/** @brief Fence handle */
	typedef GraphicsHandle<H_Fence> FenceHandle;
}

namespace std
{
	/** @brief Hash generator for DescriptorSetLayoutInfo */
	template<>
	struct hash<Glory::CommandBufferHandle>
	{
		/** @brief Hash function */
		size_t operator()(const Glory::CommandBufferHandle& handle) const noexcept
		{
			return hash<uint64_t>()(uint64_t(handle.m_ID));
		}
	};
}
