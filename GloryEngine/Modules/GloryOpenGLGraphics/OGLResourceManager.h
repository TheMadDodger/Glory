#pragma once
#include "GPUResourceManager.h"

namespace Glory
{
    class OGLResourceManager : public GPUResourceManager
    {
    public:
        OGLResourceManager(Engine* pEngine);
        virtual ~OGLResourceManager();

    protected:
        virtual Mesh* CreateMesh_Internal(MeshData* pMeshData) override;
        virtual Mesh* CreateMesh_Internal(uint32_t vertexCount, uint32_t indexCount, InputRate inputRate, uint32_t binding, uint32_t stride, PrimitiveType primitiveType, const std::vector<AttributeType>& attributeTypes) override;
        virtual Buffer* CreateVertexBuffer(uint32_t bufferSize) override;
        virtual Buffer* CreateIndexBuffer(uint32_t bufferSize) override;
        virtual Buffer* CreateBuffer_Internal(uint32_t bufferSize, BufferBindingTarget usageFlag, MemoryUsage memoryFlags, uint32_t bindIndex) override;
        virtual Shader* CreateShader_Internal(FileData* pShaderFile, const ShaderType& shaderType, const std::string& function) override;
        virtual Material* CreateMaterial_Internal(MaterialData* pMaterialData) override;
        virtual Pipeline* CreatePipeline_Internal(PipelineData* pPipelineData) override;
        virtual Texture* CreateTexture_Internal(TextureData* pTextureData) override;
        virtual Texture* CreateTexture_Internal(TextureCreateInfo&& textureInfo) override;
        virtual RenderTexture* CreateRenderTexture_Internal(const RenderTextureCreateInfo& createInfo) override;
    };
}
