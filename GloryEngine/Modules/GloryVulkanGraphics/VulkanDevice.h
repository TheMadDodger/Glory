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
        size_t m_Size;
        vk::Buffer m_VKBuffer;
        vk::DeviceMemory m_VKMemory;
        vk::BufferUsageFlags m_VKUsage;
    };

    struct VK_Mesh
    {
        vk::VertexInputBindingDescription m_VertexDescription;
        std::vector<vk::VertexInputAttributeDescription> m_AttributeDescriptions;
        uint32_t m_VertexCount;
        uint32_t m_IndexCount;
        std::vector<BufferHandle> m_Buffers;
    };

    struct VK_Texture
    {
        vk::ImageLayout m_VKLayout;
        vk::Image m_VKImage;
        vk::ImageView m_VKImageView;
        vk::DeviceMemory m_VKMemory;
        vk::Sampler m_VKSampler;
    };

    struct VK_RenderTexture
    {
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
        vk::RenderPass m_VKRenderPass;
        RenderTextureHandle m_RenderTexture;
    };

    struct VK_Shader
    {
        vk::ShaderModule m_VKModule;
        vk::ShaderStageFlagBits m_VKStage;
        std::string m_Function;
    };

    struct VK_Pipeline
    {
        RenderPassHandle m_RenderPass;
        vk::DescriptorSetLayout m_VKDescriptorSetLayouts;
        vk::PipelineLayout m_VKLayout;
        vk::DescriptorSet m_VKBuffersDescriptorSet;

        /* @todo: Create one per render pass it is being attached to? */
        vk::Pipeline m_VKPipeline;
        vk::VertexInputBindingDescription m_VertexDescription;
        std::vector<vk::VertexInputAttributeDescription> m_AttributeDescriptions;
        std::vector<ShaderHandle> m_Shaders;
    };

    struct VK_DescriptorSetLayout
    {
        vk::PushConstantRange m_PushConstantRange;
        vk::DescriptorSetLayout m_VKLayout;
        std::vector<uint32_t> m_BindingIndices;
        std::vector<vk::DescriptorType> m_DescriptorTypes;
    };

    struct VK_DescriptorSet
    {
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
        void CreateCommandBuffer();
        uint32_t VulkanDevice::GetSupportedMemoryIndex(uint32_t typeFilter, vk::MemoryPropertyFlags propertyFlags);

        GLORY_API uint32_t GraphicsFamily() const { return m_GraphicsFamily.value(); }
        GLORY_API uint32_t PresentFamily() const { return m_PresentFamily.value(); }

        GLORY_API vk::PhysicalDevice PhysicalDevice() { return m_VKDevice; }
        GLORY_API vk::Device LogicalDevice() { return m_LogicalDevice; }
        GLORY_API vk::Queue GraphicsQueue() { return m_GraphicsQueue; }
        GLORY_API vk::Queue PresentQueue() { return m_PresentQueue; }

        GLORY_API void CreateGraphicsCommandPool();
        GLORY_API vk::CommandPool CreateGraphicsCommandPool(vk::CommandPoolCreateFlags flags);

        GLORY_API vk::CommandPool GetGraphicsCommandPool();
        GLORY_API vk::CommandPool GetGraphicsCommandPool(vk::CommandPoolCreateFlags flags);

    private: /* Render commands */
        virtual void Begin() override;
        virtual void BeginRenderPass(RenderPassHandle handle) override;
        virtual void BeginPipeline(PipelineHandle handle) override;
        virtual void End() override;
        virtual void EndRenderPass() override;
        virtual void EndPipeline() override;
        virtual void BindDescriptorSets(PipelineHandle pipeline, std::vector<DescriptorSetHandle> sets, uint32_t firstSet=0) override;
        virtual void PushConstants(PipelineHandle pipeline, uint32_t offset, uint32_t size, const void* data) override;

        virtual void DrawMesh(MeshHandle handle) override;

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
        virtual DescriptorSetLayoutHandle CreateDescriptorSetLayout(const DescriptorSetLayoutInfo& setLayoutInfo) override;
        virtual DescriptorSetHandle CreateDescriptorSet(DescriptorSetInfo&& setInfo) override;

        virtual void FreeBuffer(BufferHandle& handle) override;
        virtual void FreeMesh(MeshHandle& handle) override;
        virtual void FreeTexture(TextureHandle& handle) override;
        virtual void FreeRenderTexture(RenderTextureHandle& handle) override;
        virtual void FreeRenderPass(RenderPassHandle& handle) override;
        virtual void FreeShader(ShaderHandle& handle) override;
        virtual void FreePipeline(PipelineHandle& handle) override;

    private:/* Internal */
        vk::CommandBuffer BeginSingleTimeCommands();
        void EndSingleTimeCommands(vk::CommandBuffer commandBuffer);
        void TransitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::ImageAspectFlags aspectFlags);
        void CopyFromBuffer(vk::Buffer buffer, vk::Image image, vk::ImageAspectFlags aspectFlags,
            uint32_t width, uint32_t height);
        void CopyFromBuffer(vk::Buffer buffer, vk::Image image, vk::ImageAspectFlags aspectFlags,
            int32_t offsetX, int32_t offsetY, int32_t offsetZ, uint32_t width, uint32_t height, uint32_t depth);

    private:
        vk::PhysicalDevice m_VKDevice;
        vk::PhysicalDeviceMemoryProperties m_MemoryProperties;
        std::vector<VkQueueFamilyProperties> m_AvailableQueueFamilies;
        std::vector<VkExtensionProperties> m_AvailableExtensions;
        bool m_DidLastSupportCheckPass;

        std::optional<uint32_t> m_GraphicsFamily;
        std::optional<uint32_t> m_PresentFamily;

        vk::SurfaceCapabilitiesKHR m_SwapChainCapabilities;
        std::vector<vk::SurfaceFormatKHR> m_SwapChainFormats;
        std::vector<vk::PresentModeKHR> m_SwapChainPresentModes;

        vk::Device m_LogicalDevice;
        vk::Queue m_GraphicsQueue;
        vk::Queue m_PresentQueue;

        std::vector<const char*> m_DeviceExtensions;
        vk::CommandPool m_GraphicsCommandPool;
        std::map<vk::CommandPoolCreateFlags, vk::CommandPool> m_GraphicsCommandPools;
        std::vector<vk::CommandBuffer> m_FrameCommandBuffers;

        vk::PhysicalDeviceFeatures m_Features;
        vk::PhysicalDeviceProperties m_DeviceProperties;

        vk::Fence m_Fence;

        GraphicsResources<VK_Buffer> m_Buffers;
        GraphicsResources<VK_Mesh> m_Meshes;
        GraphicsResources<VK_Texture> m_Textures;
        GraphicsResources<VK_RenderTexture> m_RenderTextures;
        GraphicsResources<VK_RenderPass> m_RenderPasses;
        GraphicsResources<VK_Shader> m_Shaders;
        GraphicsResources<VK_Pipeline> m_Pipelines;
        GraphicsResources<VK_DescriptorSetLayout> m_DescriptorSetLayouts;
        GraphicsResources<VK_DescriptorSet> m_DescriptorSets;

        std::unordered_map<SamplerSettings, vk::Sampler> m_CachedSamlers;
        std::unordered_map<DescriptorSetLayoutInfo, DescriptorSetLayoutHandle> m_CachedDescriptorSetLayouts;

        DescriptorAllocator m_DescriptorAllocator;
    };
}
