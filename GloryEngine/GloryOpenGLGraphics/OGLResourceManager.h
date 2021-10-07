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
        virtual Mesh* CreateMesh_Internal(MeshData* pMeshData);
        virtual Buffer* CreateVertexBuffer(uint32_t bufferSize) override;
        virtual Buffer* CreateIndexBuffer(uint32_t bufferSize) override;
        virtual Buffer* CreateBuffer_Internal(uint32_t bufferSize, uint32_t usageFlag, uint32_t memoryFlags) override;
    };
}
