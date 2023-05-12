#pragma once
#include "GPUResourceManager.h"

namespace Glory
{
    class OGLResourceManager : public GPUResourceManager
    {
    public:
        OGLResourceManager();
        virtual ~OGLResourceManager();

    protected:
        virtual Mesh* CreateMesh_Internal(MeshData* pMeshData) override;
        virtual Mesh* CreateMesh_Internal(uint32_t vertexCount, uint32_t indexCount, InputRate inputRate, uint32_t binding, uint32_t stride, PrimitiveType primitiveType, const std::vector<AttributeType>& attributeTypes) override;
        virtual Buffer* CreateVertexBuffer(uint32_t bufferSize) override;
        virtual Buffer* CreateIndexBuffer(uint32_t bufferSize) override;
        virtual Buffer* CreateBuffer_Internal(uint32_t bufferSize, BufferBindingTarget usageFlag, MemoryUsage memoryFlags, uint32_t bindIndex) override;
        virtual Shader* CreateShader_Internal(FileData* pShaderFile, const ShaderType& shaderType, const std::string& function) override;
        virtual Material* CreateMaterial_Internal(MaterialData* pMaterialData) override;
        virtual Texture* CreateTexture_Internal(TextureData* pTextureData) override;
        virtual Texture* CreateTexture_Internal(uint32_t width, uint32_t height, const PixelFormat& format, const PixelFormat& internalFormat, const ImageType& imageType, uint32_t usageFlags, uint32_t sharingMode, ImageAspect imageAspectFlags, const SamplerSettings& samplerSettings) override;
        virtual RenderTexture* CreateRenderTexture_Internal(const RenderTextureCreateInfo& createInfo) override;
    };
}
