#pragma once
#include "DescriptorAllocator.h"

#include <Glory.h>
#include <GraphicsDevice.h>
#include <GraphicsEnums.h>

#include <vulkan/vulkan.hpp>

#include <optional>

namespace Glory
{
    struct VK_Buffer
    {
        static constexpr GraphicsHandleType HandleType = H_Buffer;

        size_t m_Size;
        vk::Buffer m_VKBuffer;
        vk::DeviceMemory m_VKMemory;
        vk::BufferUsageFlags m_VKUsage;
    };

    struct VK_Mesh
    {
        static constexpr GraphicsHandleType HandleType = H_Mesh;

        vk::VertexInputBindingDescription m_VertexDescription;
        std::vector<vk::VertexInputAttributeDescription> m_AttributeDescriptions;
        uint32_t m_VertexCount;
        uint32_t m_IndexCount;
        std::vector<BufferHandle> m_Buffers;
    };

    struct VK_Texture
    {
        static constexpr GraphicsHandleType HandleType = H_Texture;

        vk::ImageLayout m_VKLayout;
        vk::Image m_VKImage;
        vk::ImageView m_VKImageView;
        vk::DeviceMemory m_VKMemory;
        vk::Sampler m_VKSampler;
    };

    struct VK_RenderTexture
    {
        static constexpr GraphicsHandleType HandleType = H_RenderTexture;

        vk::Framebuffer m_VKFramebuffer;
        uint32_t m_Width;
        uint32_t m_Height;
        RenderPassHandle m_RenderPass;
        std::vector<TextureHandle> m_Textures;
        std::vector<std::string> m_AttachmentNames;
        bool m_HasDepthOrStencil;
    };

    struct VK_RenderPass
    {
        static constexpr GraphicsHandleType HandleType = H_RenderPass;

        vk::RenderPass m_VKRenderPass;
        RenderTextureHandle m_RenderTexture;
    };

    struct VK_Shader
    {
        static constexpr GraphicsHandleType HandleType = H_Shader;

        vk::ShaderModule m_VKModule;
        vk::ShaderStageFlagBits m_VKStage;
        std::string m_Function;
    };

    struct VK_Pipeline
    {
        static constexpr GraphicsHandleType HandleType = H_Pipeline;

        RenderPassHandle m_RenderPass;
        vk::PipelineLayout m_VKLayout;
        vk::Pipeline m_VKPipeline;
        vk::PipelineBindPoint m_VKBindPoint;
        vk::VertexInputBindingDescription m_VertexDescription;
        std::vector<vk::VertexInputAttributeDescription> m_AttributeDescriptions;
        std::vector<ShaderHandle> m_Shaders;
    };

    struct VK_DescriptorSetLayout
    {
        static constexpr GraphicsHandleType HandleType = H_DescriptorSetLayout;

        vk::PushConstantRange m_PushConstantRange;
        vk::DescriptorSetLayout m_VKLayout;
        std::vector<uint32_t> m_BindingIndices;
        std::vector<vk::DescriptorType> m_DescriptorTypes;
    };

    struct VK_DescriptorSet
    {
        static constexpr GraphicsHandleType HandleType = H_DescriptorSet;

        DescriptorSetLayoutHandle m_Layout;
        vk::PushConstantRange m_PushConstantRange;
        vk::DescriptorSet m_VKDescriptorSet;
    };

    class VulkanGraphicsModule;

    class VulkanDevice : public GraphicsDevice
    {
    public:
        VulkanDevice(VulkanGraphicsModule* pModule, vk::PhysicalDevice physicalDevice);
        virtual ~VulkanDevice();

        GLORY_API VulkanGraphicsModule* GraphicsModule();

        void LoadData();
        void CheckSupport(std::vector<const char*> extensions);
        bool SupportCheckPassed() const;
        void CreateLogicalDevice();
        void AllocateCommandBuffers(size_t numBuffers);
        uint32_t VulkanDevice::GetSupportedMemoryIndex(uint32_t typeFilter, vk::MemoryPropertyFlags propertyFlags);

        GLORY_API uint32_t GraphicsFamily() const { return m_GraphicsAndComputeFamily.value(); }
        GLORY_API uint32_t PresentFamily() const { return m_PresentFamily.value(); }

        GLORY_API vk::PhysicalDevice PhysicalDevice() { return m_VKDevice; }
        GLORY_API vk::Device LogicalDevice() { return m_LogicalDevice; }
        GLORY_API vk::Queue GraphicsQueue() { return m_GraphicsAndComputeQueue; }
        GLORY_API vk::Queue PresentQueue() { return m_PresentQueue; }

        GLORY_API void CreateGraphicsCommandPool();
        GLORY_API vk::CommandPool CreateGraphicsCommandPool(vk::CommandPoolCreateFlags flags);

        GLORY_API vk::CommandPool GetGraphicsCommandPool();
        GLORY_API vk::CommandPool GetGraphicsCommandPool(vk::CommandPoolCreateFlags flags);

    private: /* Render commands */
        virtual CommandBufferHandle Begin() override;
        virtual void BeginRenderPass(CommandBufferHandle commandBuffer, RenderPassHandle renderPass) override;
        virtual void BeginPipeline(CommandBufferHandle commandBuffer, PipelineHandle pipeline) override;
        virtual void End(CommandBufferHandle commandBufferHandle) override;
        virtual void EndRenderPass(CommandBufferHandle commandBuffer) override;
        virtual void EndPipeline(CommandBufferHandle) override;
        virtual void BindDescriptorSets(CommandBufferHandle commandBuffer, PipelineHandle pipeline, std::vector<DescriptorSetHandle> sets, uint32_t firstSet=0) override;
        virtual void PushConstants(CommandBufferHandle commandBuffer, PipelineHandle pipeline, uint32_t offset, uint32_t size, const void* data) override;

        virtual void DrawMesh(CommandBufferHandle commandBuffer, MeshHandle handle) override;
        virtual void Dispatch(CommandBufferHandle commandBuffer, uint32_t x, uint32_t y, uint32_t z) override;
        virtual void Commit(CommandBufferHandle commandBuffer) override;
        virtual void Wait(CommandBufferHandle commandBuffer) override;
        virtual void Release(CommandBufferHandle commandBuffer) override;

    private: /* Resource management */
        virtual BufferHandle CreateBuffer(size_t bufferSize, BufferType type) override;

        virtual void AssignBuffer(BufferHandle handle, const void* data) override;
        virtual void AssignBuffer(BufferHandle handle, const void* data, uint32_t size) override;
        virtual void AssignBuffer(BufferHandle handle, const void* data, uint32_t offset, uint32_t size) override;

        virtual MeshHandle CreateMesh(std::vector<BufferHandle>&& buffers, uint32_t vertexCount,
            uint32_t indexCount, uint32_t stride, PrimitiveType primitiveType,
            const std::vector<AttributeType>& attributeTypes) override;

        virtual TextureHandle CreateTexture(TextureData* pTexture) override;
        virtual TextureHandle CreateTexture(const TextureCreateInfo& textureInfo, const void* pixels=nullptr, size_t dataSize=0) override;
        virtual RenderTextureHandle CreateRenderTexture(RenderPassHandle renderPass, const RenderTextureCreateInfo& info) override;
        virtual RenderPassHandle CreateRenderPass(const RenderPassInfo& info) override;
        virtual ShaderHandle CreateShader(const FileData* pShaderFileData, const ShaderType& shaderType, const std::string& function) override;
        virtual PipelineHandle CreatePipeline(RenderPassHandle renderPass, PipelineData* pPipeline,
            std::vector<DescriptorSetLayoutHandle>&& descriptorSetLayouts, size_t stride, const std::vector<AttributeType>& attributeTypes) override;
        virtual PipelineHandle CreateComputePipeline(PipelineData* pPipeline, std::vector<DescriptorSetLayoutHandle>&& descriptorSetLayouts) override;
        virtual DescriptorSetLayoutHandle CreateDescriptorSetLayout(DescriptorSetLayoutInfo&& setLayoutInfo) override;
        virtual DescriptorSetHandle CreateDescriptorSet(DescriptorSetInfo&& setInfo) override;

        virtual void FreeBuffer(BufferHandle& handle) override;
        virtual void FreeMesh(MeshHandle& handle) override;
        virtual void FreeTexture(TextureHandle& handle) override;
        virtual void FreeRenderTexture(RenderTextureHandle& handle) override;
        virtual void FreeRenderPass(RenderPassHandle& handle) override;
        virtual void FreeShader(ShaderHandle& handle) override;
        virtual void FreePipeline(PipelineHandle& handle) override;
        virtual void FreeDescriptorSetLayout(DescriptorSetLayoutHandle& handle) override;
        virtual void FreeDescriptorSet(DescriptorSetHandle& handle) override;

    private:/* Internal */
        vk::CommandBuffer BeginSingleTimeCommands();
        void EndSingleTimeCommands(vk::CommandBuffer commandBuffer);
        void TransitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::ImageAspectFlags aspectFlags, uint32_t mipLevels);
        void GenerateMipMaps(vk::Image image, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
        void CopyFromBuffer(vk::Buffer buffer, vk::Image image, vk::ImageAspectFlags aspectFlags,
            uint32_t width, uint32_t height);
        void CopyFromBuffer(vk::Buffer buffer, vk::Image image, vk::ImageAspectFlags aspectFlags,
            int32_t offsetX, int32_t offsetY, int32_t offsetZ, uint32_t width, uint32_t height, uint32_t depth);

        vk::CommandBuffer GetNewCommandBuffer(CommandBufferHandle commandBufferHandle);

    private:
        vk::PhysicalDevice m_VKDevice;
        vk::PhysicalDeviceMemoryProperties m_MemoryProperties;
        std::vector<VkQueueFamilyProperties> m_AvailableQueueFamilies;
        std::vector<VkExtensionProperties> m_AvailableExtensions;
        bool m_DidLastSupportCheckPass;

        std::optional<uint32_t> m_GraphicsAndComputeFamily;
        std::optional<uint32_t> m_PresentFamily;

        vk::SurfaceCapabilitiesKHR m_SwapChainCapabilities;
        std::vector<vk::SurfaceFormatKHR> m_SwapChainFormats;
        std::vector<vk::PresentModeKHR> m_SwapChainPresentModes;

        vk::Device m_LogicalDevice;
        vk::Queue m_GraphicsAndComputeQueue;
        vk::Queue m_PresentQueue;

        std::vector<const char*> m_DeviceExtensions;
        vk::CommandPool m_GraphicsCommandPool;
        std::map<vk::CommandPoolCreateFlags, vk::CommandPool> m_GraphicsCommandPools;

        vk::PhysicalDeviceFeatures m_Features;
        vk::PhysicalDeviceProperties m_DeviceProperties;

        GraphicsResources<VK_Buffer> m_Buffers;
        GraphicsResources<VK_Mesh> m_Meshes;
        GraphicsResources<VK_Texture> m_Textures;
        GraphicsResources<VK_RenderTexture> m_RenderTextures;
        GraphicsResources<VK_RenderPass> m_RenderPasses;
        GraphicsResources<VK_Shader> m_Shaders;
        GraphicsResources<VK_Pipeline> m_Pipelines;
        GraphicsResources<VK_DescriptorSetLayout> m_DescriptorSetLayouts;
        GraphicsResources<VK_DescriptorSet> m_DescriptorSets;

        std::unordered_map<UUID, vk::CommandBuffer> m_CommandBuffers;
        std::unordered_map<UUID, vk::Fence> m_CommandBufferFences;
        std::vector<vk::CommandBuffer> m_FreeCommandBuffers;
        std::vector<vk::Fence> m_FreeFences;

        std::unordered_map<SamplerSettings, vk::Sampler> m_CachedSamlers;
        std::unordered_map<DescriptorSetLayoutInfo, DescriptorSetLayoutHandle> m_CachedDescriptorSetLayouts;

        DescriptorAllocator m_DescriptorAllocator;
    };
}
