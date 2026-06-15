#pragma once

namespace Glory
{
    struct GL_CommandBuffer;
    struct GL_CommandData;
    class OpenGLDevice;

    /** @brief Implementations for OpenGL commands */
    class OpenGLCommandImpl
    {
    public:
        static void Command_Impl(OpenGLDevice& device, const GL_CommandBuffer& commandBuffer, const GL_CommandData& data);
        static void Begin_Impl(OpenGLDevice& device, const GL_CommandBuffer& commandBuffer, const GL_CommandData& data);
        static void BeginRenderPass_Impl(OpenGLDevice& device, const GL_CommandBuffer& commandBuffer, const GL_CommandData& data);
        static void BeginPipeline_Impl(OpenGLDevice& device, const GL_CommandBuffer& commandBuffer, const GL_CommandData& data);
        static void End_Impl(OpenGLDevice& device, const GL_CommandBuffer& commandBuffer, const GL_CommandData& data);
        static void EndRenderPass_Impl(OpenGLDevice& device, const GL_CommandBuffer& commandBuffer, const GL_CommandData& data);
        static void EndPipeline_Impl(OpenGLDevice& device, const GL_CommandBuffer& commandBuffer, const GL_CommandData& data);
        static void BindDescriptorSets_Impl(OpenGLDevice& device, const GL_CommandBuffer& commandBuffer, const GL_CommandData& data);
        static void PushConstants_Impl(OpenGLDevice& device, const GL_CommandBuffer& commandBuffer, const GL_CommandData& data);
        static void DrawMesh_Impl(OpenGLDevice& device, const GL_CommandBuffer& commandBuffer, const GL_CommandData& data);
        static void Dispatch_Impl(OpenGLDevice& device, const GL_CommandBuffer& commandBuffer, const GL_CommandData& data);
        static void SetStencilTestEnabled_Impl(OpenGLDevice& device, const GL_CommandBuffer& commandBuffer, const GL_CommandData& data);
        static void SetStencilOp_Impl(OpenGLDevice& device, const GL_CommandBuffer& commandBuffer, const GL_CommandData& data);
        static void SetStencilWriteMask_Impl(OpenGLDevice& device, const GL_CommandBuffer& commandBuffer, const GL_CommandData& data);
        static void Commit_Impl(OpenGLDevice& device, const GL_CommandBuffer& commandBuffer);
        static void SetViewport_Impl(OpenGLDevice& device, const GL_CommandBuffer& commandBuffer, const GL_CommandData& data);
        static void SetScissor_Impl(OpenGLDevice& device, const GL_CommandBuffer& commandBuffer, const GL_CommandData& data);
        static void PipelineBarrier_Impl(OpenGLDevice& device, const GL_CommandBuffer& commandBuffer, const GL_CommandData& data);
        static void CopyImage_Impl(OpenGLDevice& device, const GL_CommandBuffer& commandBuffer, const GL_CommandData& data);
        static void CopyImageToBuffer_Impl(OpenGLDevice& device, const GL_CommandBuffer& commandBuffer, const GL_CommandData& data);
    };
}
