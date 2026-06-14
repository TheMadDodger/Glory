#pragma once
#include "ogl_visibility.h"

#include <GraphicsDevice.h>

#include <BitSet.h>

#include <queue>

typedef struct __GLsync* GLsync;

namespace Glory
{
    inline constexpr size_t PushConstantsMaxSize = 128;

    struct GL_Buffer
    {
        static constexpr GraphicsHandleType HandleType = H_Buffer;

        size_t m_Size;
        uint32_t m_GLBufferID;
        uint32_t m_GLTarget;
        uint32_t m_GLUsage;
    };

    struct GL_Mesh
    {
        static constexpr GraphicsHandleType HandleType = H_Mesh;

        uint32_t m_GLVertexArrayID;
        uint32_t m_VertexCount;
        uint32_t m_IndexCount;
        std::vector<BufferHandle> m_Buffers;
    };

    struct GL_Texture
    {
        static constexpr GraphicsHandleType HandleType = H_Texture;

        uint32_t m_Width;
        uint32_t m_Height;
        uint32_t m_GLTextureID;
        uint32_t m_GLTextureType;
        uint32_t m_GLFormat;
        uint32_t m_GLInternalFormat;
        uint32_t m_GLDataType;
        uint64_t m_GLBindlessHandle;

        uint32_t m_GLSamplerID;
        int m_GLMinFilter;
        int m_GLMagFilter;
        int m_GLTextureWrapS;
        int m_GLTextureWrapT;
        int m_GLTextureWrapR;
    };

    struct GL_RenderTexture
    {
        static constexpr GraphicsHandleType HandleType = H_RenderTexture;

        uint32_t m_GLFramebufferID;
        RenderPassHandle m_RenderPass;
        std::vector<TextureHandle> m_Textures;
        std::vector<std::string> m_AttachmentNames;
        RenderTextureCreateInfo m_Info;
    };

    struct GL_RenderPass
    {
        static constexpr GraphicsHandleType HandleType = H_RenderPass;

        RenderTextureHandle m_RenderTexture;
        glm::vec4 m_ClearColor;
        float m_DepthClear;
        uint8_t m_StencilClear;
        bool m_Clear;
    };

    struct GL_Shader
    {
        static constexpr GraphicsHandleType HandleType = H_Shader;

        uint32_t m_GLShaderID;
        uint32_t m_GLShaderType;
    };

    struct GL_Pipeline
    {
        static constexpr GraphicsHandleType HandleType = H_Pipeline;

        std::vector<ShaderHandle> m_Shaders;

        RenderPassHandle m_RenderPass;
        uint32_t m_GLProgramID;
        uint32_t m_GLCullFace;
        uint32_t m_GLPrimitiveType;
        uint32_t m_GLDepthFunc;
        uint32_t m_GLStencilCompareOp;
        uint32_t m_GLStencilFailOp;
        uint32_t m_GLStencilDepthFailOp;
        uint32_t m_GLStencilPassOp;
        uint32_t m_GLSrcColorBlendFactor;
        uint32_t m_GLDstColorBlendFactor;
        uint32_t m_GLColorBlendOp;
        uint32_t m_GLSrcAlphaBlendFactor;
        uint32_t m_GLDstAlphaBlendFactor;
        uint32_t m_GLAlphaBlendOp;
        glm::vec4 m_BlendConstants = glm::vec4{};
        Utils::BitSet m_SettingToggles;
    };

    struct GL_DescriptorSetLayout
    {
        static constexpr GraphicsHandleType HandleType = H_DescriptorSetLayout;

        std::vector<uint32_t> m_BindingIndices;
        std::vector<std::string> m_SamplerNames;
        std::vector<uint32_t> m_DescriptorCounts;
    };

    struct GL_DescriptorSet
    {
        static constexpr GraphicsHandleType HandleType = H_DescriptorSet;

        DescriptorSetLayoutHandle m_Layout;
        std::vector<BufferHandle> m_Buffers;
        std::vector<TextureHandle> m_Textures;
        std::vector<BufferHandle> m_BindlessTexturesBuffers;
    };

    struct GL_Swapchain
    {
        static constexpr GraphicsHandleType HandleType = H_Swapchain;

        Window* m_pWindow;
        std::vector<RenderTextureHandle> m_SwapchainImages;
        uint32_t m_CurrentImageIndex = 0;
    };

    enum class GLCommandType : uint8_t
    {
        Unknown,
        Begin,
        BeginRenderPass,
        BeginPipeline,
        End,
        EndRenderPass,
        EndPipeline,
        BindDescriptorSets,
        PushConstants,
        DrawMesh,
        Dispatch,
        SetStencilTestEnabled,
        SetStencilOp,
        SetStencilWriteMask,
        SetViewport,
        SetScissor,
        PipelineBarrier,
        CopyImage,
        CopyImageToBuffer
    };

    struct GL_CommandData
    {
        GL_CommandData(): m_CommandType(GLCommandType::Unknown) {}
        GL_CommandData(const GLCommandType& commandType): m_CommandType(commandType) {}
        ~GL_CommandData()
        {
            m_RenderPass = nullptr;
        }

        GL_CommandData(GL_CommandData&& other) noexcept
        {
            m_CommandType = other.m_CommandType;
            m_RenderPass = std::move(other.m_RenderPass);
            m_XYZ = std::move(other.m_XYZ);
        }
        GL_CommandData& operator=(GL_CommandData&& other) noexcept
        {
            m_CommandType = other.m_CommandType;
            m_RenderPass = std::move(other.m_RenderPass);
            m_XYZ = std::move(other.m_XYZ);
            return *this;
        }

        GLCommandType m_CommandType;

        union
        {
            /* Renderpass commands */
            RenderPassHandle m_RenderPass = nullptr;

            /* Pipeline commands */
            PipelineHandle m_Pipeline;

            /* Mesh commands */
            MeshHandle m_Mesh;

            /* Stencil/depth test commands */
            struct
            {
                uint8_t m_Enable;
                uint8_t m_CompareOp;
                uint8_t m_Fail;
                uint8_t m_DepthFail;
                uint8_t m_Pass;
                int8_t m_Reference;
                uint8_t m_Mask;
                uint8_t m_StencilDepthPadding;
            };

            struct
            {
                uint32_t m_FlagBits;
                uint32_t m_FlagBitsPadding;
            };
        };
        union
        {
            glm::uvec4 m_XYZ = glm::uvec4{ 0 };

            glm::uvec4 m_XYZSigned;

            glm::vec4 m_XYZFloat;

            /* Descriptor set */
            struct
            {
                DescriptorSetHandle m_DescriptorSet;
                uint64_t m_DescriptorSetPadding;
            };

            /* Texture/image commands */
            struct
            {
                TextureHandle m_SrcTexture;
                union
                {
                    TextureHandle m_DstTexture;
                    BufferHandle m_DstBuffer;
                };
            };

            /* Push constants */
            struct
            {
                uint32_t m_PushConstantsOffset;
                uint32_t m_PushConstantsSize;
                uint32_t m_PushConstantsDataIndex;
                uint32_t m_PushConstantsPadding;
            };
        };
    };

    struct GL_CommandBuffer
    {
        static constexpr GraphicsHandleType HandleType = H_CommandBuffer;

        GL_CommandBuffer(size_t capacity = 32);

        GLsync m_Fence = nullptr;
        size_t m_CommandsSize = 0;
        size_t m_CommandsCapacity;
        mutable uint32_t m_GLCurrentPrimitives = 0;

        std::unique_ptr<GL_CommandData[]> m_Commands;
        std::vector<std::array<char, PushConstantsMaxSize>> m_PushConstantData;
    };

    class OpenGLGraphicsModule;

    class OpenGLDevice : public GraphicsDevice
    {
    public:
        OpenGLDevice(OpenGLGraphicsModule* pModule);
        virtual ~OpenGLDevice();

        OpenGLGraphicsModule* GraphicsModule();

        GLORY_OGL_API uint32_t GetGLTextureID(TextureHandle texture);

    private: /* Render commands */
        virtual CommandBufferHandle CreateCommandBuffer() override;
        virtual void Begin(CommandBufferHandle commandBuffer) override;
        virtual void BeginRenderPass(CommandBufferHandle commandBuffer, RenderPassHandle renderPass) override;
        virtual void BeginPipeline(CommandBufferHandle commandBuffer, PipelineHandle pipeline) override;
        virtual void End(CommandBufferHandle commandBuffer) override;
        virtual void EndRenderPass(CommandBufferHandle commandBuffer) override;
        virtual void EndPipeline(CommandBufferHandle commandBuffer) override;
        virtual void BindDescriptorSets(CommandBufferHandle commandBuffer, PipelineHandle pipeline, const std::vector<DescriptorSetHandle>& sets, uint32_t firstSet=0) override;
        virtual void PushConstants(CommandBufferHandle commandBuffer, PipelineHandle pipeline, uint32_t offset, uint32_t size, const void* data, ShaderTypeFlag) override;

        virtual void DrawMesh(CommandBufferHandle commandBuffer, MeshHandle handle) override;
        virtual void Dispatch(CommandBufferHandle commandBuffer, uint32_t x, uint32_t y, uint32_t z) override;

        virtual void SetStencilTestEnabled(CommandBufferHandle commandBuffer, bool enable) override;
        virtual void SetStencilOp(CommandBufferHandle commandBuffer, CompareOp compareOp,
            Func fail, Func depthFail, Func pass, int8_t reference, uint8_t compareMask) override;
        virtual void SetStencilWriteMask(CommandBufferHandle commandBuffer, uint8_t mask) override;

        virtual void Commit(CommandBufferHandle commandBuffer, const std::vector<SemaphoreHandle>&,
            const std::vector<SemaphoreHandle>&) override;
        virtual WaitResult Wait(CommandBufferHandle commandBuffer, uint64_t timeout) override;
        virtual void Release(CommandBufferHandle commandBuffer) override;
        virtual void Reset(CommandBufferHandle commandBuffer) override;

        virtual void SetViewport(CommandBufferHandle commandBuffer, float x, float y, float width, float height, float minDepth=0.0f, float maxDepth=1.0f) override;
        virtual void SetScissor(CommandBufferHandle commandBuffer, int x, int y, uint32_t width, uint32_t height) override;

        virtual void PipelineBarrier(CommandBufferHandle, const std::vector<BufferBarrier>& buffers,
            const std::vector<ImageBarrier>& images, PipelineStageFlagBits, PipelineStageFlagBits) override;
        virtual void CopyImage(CommandBufferHandle commandBuffer, TextureHandle src, TextureHandle dst) override;
        virtual void CopyImageToBuffer(CommandBufferHandle commandBuffer, TextureHandle src, BufferHandle dst) override;

        virtual SwapchainResult AcquireNextSwapchainImage(SwapchainHandle swapchain, uint32_t* imageIndex, SemaphoreHandle) override;
        virtual SwapchainResult Present(SwapchainHandle swapchain, uint32_t imageIndex, const std::vector<SemaphoreHandle>& waitSemaphores={}) override;

        virtual void WaitIdle() override;

    private: /* Resource management */
        virtual BufferHandle CreateBuffer(size_t bufferSize, BufferType type, BufferFlags flags=BF_None) override;
        virtual void ResizeBuffer(BufferHandle buffer, size_t bufferSize) override;
        virtual size_t BufferSize(BufferHandle buffer) override;

        virtual void AssignBuffer(BufferHandle handle, const void* data) override;
        virtual void AssignBuffer(BufferHandle handle, const void* data, uint32_t size) override;
        virtual void AssignBuffer(BufferHandle handle, const void* data, uint32_t offset, uint32_t size) override;
        virtual void ReadBuffer(BufferHandle handle, void* outData, uint32_t offset, uint32_t size) override;

        virtual MeshHandle CreateMesh(std::vector<BufferHandle>&& buffers, uint32_t vertexCount,
            uint32_t indexCount, uint32_t stride, const std::vector<AttributeType>& attributeTypes) override;
        virtual void UpdateMesh(MeshHandle mesh, std::vector<BufferHandle>&& buffers,
            uint32_t vertexCount, uint32_t indexCount) override;
        virtual void UpdateMesh(MeshHandle texture, MeshData* pMeshData) override;

        virtual TextureHandle CreateTexture(TextureData* pTexture) override;
        virtual TextureHandle CreateTexture(CubemapData* pCubemap) override;
        virtual TextureHandle CreateTexture(const TextureCreateInfo& textureInfo, const void* pixels=nullptr, size_t dataSize=0) override;
        virtual void UpdateTexture(TextureHandle texture, TextureData* pTextureData) override;
        virtual void ReadTexturePixels(TextureHandle texture, void* dst, size_t offset, size_t size) override;
        virtual uint64_t GetTextureBindlessHandle(TextureHandle texture) override;

        virtual RenderTextureHandle CreateRenderTexture(RenderPassHandle renderPass, RenderTextureCreateInfo&& info) override;
        virtual TextureHandle GetRenderTextureAttachment(RenderTextureHandle renderTexture, size_t index) override;
        virtual void ResizeRenderTexture(RenderTextureHandle renderTexture, uint32_t width, uint32_t height) override;
        virtual RenderPassHandle CreateRenderPass(RenderPassInfo&& info) override;
        virtual RenderTextureHandle GetRenderPassRenderTexture(RenderPassHandle renderPass) override;
        virtual void SetRenderPassClear(RenderPassHandle renderPass, const glm::vec4& color, float depth=1.0f, uint8_t stencil=0) override;
        virtual ShaderHandle CreateShader(const FileData* pShaderFileData, const ShaderType& shaderType, const std::string& function) override;
        virtual PipelineHandle CreatePipeline(RenderPassHandle renderPass, PipelineData* pPipeline,
            std::vector<DescriptorSetLayoutHandle>&&, size_t, const std::vector<AttributeType>&) override;
        virtual void UpdatePipelineSettings(PipelineHandle pipeline, PipelineData* pPipeline) override;
        virtual void RecreatePipeline(PipelineHandle pipeline, PipelineData* pPipeline) override;
        virtual PipelineHandle CreateComputePipeline(PipelineData* pPipeline, std::vector<DescriptorSetLayoutHandle>&& descriptorSetLayouts) override;
        virtual DescriptorSetLayoutHandle CreateDescriptorSetLayout(DescriptorSetLayoutInfo&& setLayoutInfo) override;
        virtual DescriptorSetHandle CreateDescriptorSet(DescriptorSetInfo&& setInfo) override;
        virtual void UpdateDescriptorSet(DescriptorSetHandle descriptorSet, const DescriptorSetUpdateInfo& setWriteInfo) override;
        virtual SwapchainHandle CreateSwapchain(Window* pWindow, bool vsync=false, uint32_t minImageCount=0) override;
        virtual uint32_t GetSwapchainImageCount(SwapchainHandle swapchain) override;
        virtual TextureHandle GetSwapchainImage(SwapchainHandle swapchain, uint32_t imageIndex) override;
        virtual void RecreateSwapchain(SwapchainHandle swapchain) override;
        virtual SemaphoreHandle CreateSemaphore() override;

        virtual void FreeBuffer(BufferHandle& handle) override;
        virtual void FreeMesh(MeshHandle& handle) override;
        virtual void FreeTexture(TextureHandle& handle) override;
        virtual void FreeRenderTexture(RenderTextureHandle& handle) override;
        virtual void FreeRenderPass(RenderPassHandle& handle) override;
        virtual void FreeShader(ShaderHandle& handle) override;
        virtual void FreePipeline(PipelineHandle& handle) override;
        virtual void FreeDescriptorSetLayout(DescriptorSetLayoutHandle& handle) override;
        virtual void FreeDescriptorSet(DescriptorSetHandle& handle) override;
        virtual void FreeSwapchain(SwapchainHandle& handle) override;
        virtual void FreeSemaphore(SemaphoreHandle& handle) override;

        virtual void OnInitialize() override;

    private:
        void CreateRenderTexture(GL_RenderTexture& renderTexture);
        bool CreatePipeline(GL_Pipeline& pipeline, PipelineData* pPipeline);
        void PushCommand(GL_CommandBuffer& buffer, GL_CommandData&& commandData);

    private:
        void Begin_Impl(const GL_CommandBuffer& commandBuffer, const GL_CommandData& data);
        void BeginRenderPass_Impl(const GL_CommandBuffer& commandBuffer, const GL_CommandData& data);
        void BeginPipeline_Impl(const GL_CommandBuffer& commandBuffer, const GL_CommandData& data);
        void End_Impl(const GL_CommandBuffer& commandBuffer, const GL_CommandData& data);
        void EndRenderPass_Impl(const GL_CommandBuffer& commandBuffer, const GL_CommandData& data);
        void EndPipeline_Impl(const GL_CommandBuffer& commandBuffer, const GL_CommandData& data);
        void BindDescriptorSets_Impl(const GL_CommandBuffer& commandBuffer, const GL_CommandData& data);
        void PushConstants_Impl(const GL_CommandBuffer& commandBuffer, const GL_CommandData& data);
        void DrawMesh_Impl(const GL_CommandBuffer& commandBuffer, const GL_CommandData& data);
        void Dispatch_Impl(const GL_CommandBuffer& commandBuffer, const GL_CommandData& data);
        void SetStencilTestEnabled_Impl(const GL_CommandBuffer& commandBuffer, const GL_CommandData& data);
        void SetStencilOp_Impl(const GL_CommandBuffer& commandBuffer, const GL_CommandData& data);
        void SetStencilWriteMask_Impl(const GL_CommandBuffer& commandBuffer, const GL_CommandData& data);
        void Commit_Impl(const GL_CommandBuffer& commandBuffer);
        void SetViewport_Impl(const GL_CommandBuffer& commandBuffer, const GL_CommandData& data);
        void SetScissor_Impl(const GL_CommandBuffer& commandBuffer, const GL_CommandData& data);
        void PipelineBarrier_Impl(const GL_CommandBuffer& commandBuffer, const GL_CommandData& data);
        void CopyImage_Impl(const GL_CommandBuffer& commandBuffer, const GL_CommandData& data);
        void CopyImageToBuffer_Impl(const GL_CommandBuffer& commandBuffer, const GL_CommandData& data);

    private:
        GraphicsResources<GL_Buffer> m_Buffers;
        GraphicsResources<GL_Mesh> m_Meshes;
        GraphicsResources<GL_Texture> m_Textures;
        GraphicsResources<GL_RenderTexture> m_RenderTextures;
        GraphicsResources<GL_RenderPass> m_RenderPasses;
        GraphicsResources<GL_Shader> m_Shaders;
        GraphicsResources<GL_Pipeline> m_Pipelines;
        GraphicsResources<GL_DescriptorSetLayout> m_SetLayouts;
        GraphicsResources<GL_DescriptorSet> m_Sets;
        GraphicsResources<GL_Swapchain> m_Swapchains;
        GraphicsResources<GL_CommandBuffer> m_CommandBuffers;
        std::queue<CommandBufferHandle> m_FreeCommandBuffers;
        std::unordered_map<DescriptorSetLayoutInfo, DescriptorSetLayoutHandle> m_CachedDescriptorSetLayouts;

        /* For push constant emulation */
        BufferHandle m_ConstantsBuffer;
    };
}
