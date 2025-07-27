#pragma once
#include <GraphicsDevice.h>

namespace Glory
{
    struct GL_Buffer
    {
        size_t m_Size;
        uint32_t m_GLBufferID;
        uint32_t m_Target;
        uint32_t m_Usage;
    };

    struct GL_Mesh
    {
        uint32_t m_GLVertexArrayID;
        std::vector<BufferHandle> m_Buffers;
    };

    class OpenGLGraphicsModule;

    class OpenGLDevice : public GraphicsDevice
    {
    public:
        OpenGLDevice(OpenGLGraphicsModule* pModule);
        virtual ~OpenGLDevice();

        OpenGLGraphicsModule* GraphicsModule();

    private: /* Resource management */
        virtual BufferHandle CreateBuffer(size_t bufferSize, BufferType type) override;

        virtual void AssignBuffer(BufferHandle handle, const void* data) override;
        virtual void AssignBuffer(BufferHandle handle, const void* data, uint32_t size) override;
        virtual void AssignBuffer(BufferHandle handle, const void* data, uint32_t offset, uint32_t size) override;

        virtual MeshHandle CreateMesh(std::vector<BufferHandle>&& buffers, uint32_t vertexCount,
            uint32_t indexCount, uint32_t stride, PrimitiveType primitiveType,
            const std::vector<AttributeType>& attributeTypes) override;

        virtual void FreeBuffer(BufferHandle& handle) override;
        virtual void FreeMesh(MeshHandle& handle) override;

    private:
        GraphicsResources<GL_Buffer> m_Buffers;
        GraphicsResources<GL_Mesh> m_Meshes;
    };
}
