#pragma once
#include "Module.h"
#include "GPUResourceManager.h"

namespace Glory
{
    class GraphicsModule : public Module
    {
    public:
        GraphicsModule();
        virtual ~GraphicsModule();

        virtual const std::type_info& GetModuleType() override;

        int GetLastDrawCalls();
        int GetLastVertexCount();
        int GetLastTriangleCount();

    public: // Commands
        void DrawMesh(MeshData* pMeshData, uint32_t vertexOffset, uint32_t vertexCount);
        void DrawMesh(Mesh* pMesh, uint32_t vertexOffset, uint32_t vertexCount);
        void MultiDrawMeshIndirect(Mesh* pMesh, size_t count);
        virtual void DrawScreenQuad() = 0;
        virtual void DrawUnitCube() = 0;
        virtual void Clear(glm::vec4 color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), double depth=1.0) = 0;
        virtual void Swap() = 0;
        virtual Material* UseMaterial(MaterialData* pMaterialData) = 0;
        virtual void DispatchCompute(size_t num_groups_x, size_t num_groups_y, size_t num_groups_z) = 0;
        virtual void EnableDepthTest(bool enable) = 0;
        virtual void EnableDepthWrite(bool enable) = 0;
        virtual void EnableStencilTest(bool enable) = 0;
        virtual void SetStencilMask(unsigned int mask) = 0;
        virtual void SetStencilFunc(CompareOp func, int ref, unsigned int mask) = 0;
        virtual void SetStencilOP(Func fail, Func dpfail, Func dppass) = 0;
        virtual void SetColorMask(bool r, bool g, bool b, bool a) = 0;
        virtual void ClearStencil(int value) = 0;
        virtual void SetViewport(int x, int y, uint32_t width, uint32_t height) = 0;
        virtual void Scissor(int x, int y, uint32_t width, uint32_t height) = 0;
        virtual void EndScissor() = 0;
        virtual void Blit(RenderTexture* pTexture, glm::uvec4 src = glm::uvec4(), glm::uvec4 dst = glm::uvec4(),
            Filter filter = Filter::F_Nearest) = 0;
        virtual void Blit(RenderTexture* pSource, RenderTexture* pDest, glm::uvec4 src = glm::uvec4(), glm::uvec4 dst = glm::uvec4(),
            Filter filter = Filter::F_Nearest) = 0;
        virtual void SetCullFace(CullFace cullFace) = 0;

        virtual Material* UsePassthroughMaterial() { return nullptr; };

    public: // Getters
        GPUResourceManager* GetResourceManager();

    protected:
        virtual void OnInitialize() = 0;
        virtual void OnCleanup() = 0;

        virtual void OnDrawMesh(Mesh* pMesh, uint32_t vertexOffset = 0, uint32_t vertexCount = 0) = 0;
        virtual void OnMultiDrawMeshIndirect(Mesh* pMesh, size_t count) = 0;

        virtual GPUResourceManager* CreateGPUResourceManager() = 0;

    private:
        virtual void Initialize() override;
        virtual void Cleanup() override;

        virtual void OnBeginFrame() override;
        virtual void OnEndFrame() override;

    private:
        GPUResourceManager* m_pResourceManager;

        int m_CurrentDrawCalls;
        int m_LastDrawCalls;
        int m_LastVertices;
        int m_CurrentVertices;
        int m_LastTriangles;
        int m_CurrentTriangles;
    };
}