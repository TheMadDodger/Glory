#pragma once
#include <Glory.h>
#include <GraphicsDevice.h>

#include <vulkan/vulkan.hpp>

#include <optional>

namespace Glory
{
    struct VK_Buffer
    {
        size_t m_Size;
        uint32_t m_GLBufferID;
        uint32_t m_GLTarget;
        uint32_t m_GLUsage;
    };

    struct VK_Mesh
    {
        uint32_t m_GLVertexArrayID;
        uint32_t m_GLPrimitiveType;
        uint32_t m_VertexCount;
        uint32_t m_IndexCount;
        std::vector<BufferHandle> m_Buffers;
    };

    struct VK_Texture
    {
        uint32_t m_GLTextureID;
        uint32_t m_GLTextureType;
        uint32_t m_GLFormat;
        uint32_t m_GLInternalFormat;
        uint32_t m_GLDataType;

        int m_GLMinFilter;
        int m_GLMagFilter;
        int m_GLTextureWrapS;
        int m_GLTextureWrapT;
        int m_GLTextureWrapR;
    };

    struct VK_RenderTexture
    {
        uint32_t m_GLFramebufferID;
        uint32_t m_Width;
        uint32_t m_Height;
        RenderPassHandle m_RenderPass;
        std::vector<TextureHandle> m_Textures;
        std::vector<std::string> m_AttachmentNames;
    };

    struct VK_RenderPass
    {
        RenderTextureHandle m_RenderTexture;
    };

    struct VK_Shader
    {
        uint32_t m_GLShaderID;
        uint32_t m_GLShaderType;
    };

    struct VK_Pipeline
    {
        RenderPassHandle m_RenderPass;
        uint32_t m_GLProgramID;
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
        virtual void BeginRenderPass(RenderPassHandle handle) override;
        virtual void BeginPipeline(PipelineHandle handle) override;
        virtual void EndRenderPass() override;
        virtual void EndPipeline() override;

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
        virtual TextureHandle CreateTexture(const TextureCreateInfo& textureInfo, const void* pixels = nullptr) override;
        virtual RenderTextureHandle CreateRenderTexture(RenderPassHandle renderPass, const RenderTextureCreateInfo& info) override;
        virtual RenderPassHandle CreateRenderPass(const RenderPassInfo& info) override;
        virtual ShaderHandle CreateShader(const FileData* pShaderFileData, const ShaderType& shaderType, const std::string& function) override;
        virtual PipelineHandle CreatePipeline(RenderPassHandle renderPass, PipelineData* pPipeline) override;

        virtual void FreeBuffer(BufferHandle& handle) override;
        virtual void FreeMesh(MeshHandle& handle) override;
        virtual void FreeTexture(TextureHandle& handle) override;
        virtual void FreeRenderTexture(RenderTextureHandle& handle) override;
        virtual void FreeRenderPass(RenderPassHandle& handle) override;

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
        vk::PhysicalDeviceFeatures m_Features;
        vk::PhysicalDeviceProperties m_DeviceProperties;

        GraphicsResources<VK_Buffer> m_Buffers;
        GraphicsResources<VK_Mesh> m_Meshes;
        GraphicsResources<VK_Texture> m_Textures;
        GraphicsResources<VK_RenderTexture> m_RenderTextures;
        GraphicsResources<VK_RenderPass> m_RenderPasses;
        GraphicsResources<VK_Shader> m_Shaders;
        GraphicsResources<VK_Pipeline> m_Pipelines;
    };
}
