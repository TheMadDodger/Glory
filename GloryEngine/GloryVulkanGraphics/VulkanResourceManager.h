#pragma once
#include <GPUResourceManager.h>

namespace Glory
{
    class VulkanResourceManager : public GPUResourceManager
    {
    public:
        VulkanResourceManager();
        virtual ~VulkanResourceManager();

    protected:
        virtual Mesh* CreateMesh_Internal(MeshData* pMeshData) override;
        virtual Buffer* CreateVertexBuffer(uint32_t bufferSize) override;
        virtual Buffer* CreateIndexBuffer(uint32_t bufferSize) override;
        virtual Buffer* CreateBuffer_Internal(uint32_t bufferSize, uint32_t usageFlag, uint32_t memoryFlags) override;
        virtual Shader* CreateShader_Internal(FileData* pShaderFile, const ShaderType& shaderType, const std::string& function) override;
        virtual Material* CreateMaterial_Internal(MaterialData* pMaterialData) override;
        virtual Texture* CreateTexture_Internal(ImageData* pImageData) override;
    };
}
