#pragma once
#include <GraphicsDevice.h>

namespace Glory
{
    struct GL_Buffer
    {
        size_t m_Size;
        uint32_t m_GLBufferID;
        uint32_t m_GLTarget;
        uint32_t m_GLUsage;
    };

    struct GL_Mesh
    {
        uint32_t m_GLVertexArrayID;
        uint32_t m_GLPrimitiveType;
        uint32_t m_VertexCount;
        uint32_t m_IndexCount;
        std::vector<BufferHandle> m_Buffers;
    };

    struct GL_Texture
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

    struct GL_RenderTexture
    {
        uint32_t m_GLFramebufferID;
        uint32_t m_Width;
        uint32_t m_Height;
        RenderPassHandle m_RenderPass;
        std::vector<TextureHandle> m_Textures;
        std::vector<std::string> m_AttachmentNames;
    };

    struct GL_RenderPass
    {
        RenderTextureHandle m_RenderTexture;
    };

    struct GL_Shader
    {
        uint32_t m_GLShaderID;
        uint32_t m_GLShaderType;
    };

    struct GL_Pipeline
    {
        RenderPassHandle m_RenderPass;
        uint32_t m_GLProgramID;
    };

    class OpenGLGraphicsModule;

    class OpenGLDevice : public GraphicsDevice
    {
    public:
        OpenGLDevice(OpenGLGraphicsModule* pModule);
        virtual ~OpenGLDevice();

        OpenGLGraphicsModule* GraphicsModule();

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
        GraphicsResources<GL_Buffer> m_Buffers;
        GraphicsResources<GL_Mesh> m_Meshes;
        GraphicsResources<GL_Texture> m_Textures;
        GraphicsResources<GL_RenderTexture> m_RenderTextures;
        GraphicsResources<GL_RenderPass> m_RenderPasses;
        GraphicsResources<GL_Shader> m_Shaders;
        GraphicsResources<GL_Pipeline> m_Pipelines;
    };
}
