#pragma once
#include "GLMesh.h"

#include <GraphicsModule.h>
#include <GL/glew.h>

namespace Glory
{
	class OpenGLGraphicsModule : public GraphicsModule
	{
	public:
		OpenGLGraphicsModule();
		virtual ~OpenGLGraphicsModule();
		static void LogGLError(const GLenum& err, bool bIncludeTimeStamp = true);

		GLORY_MODULE_VERSION_H(0,1,0);

	public: // Commands
		virtual void Clear(glm::vec4 color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)) override;
		virtual void Swap() override;
		virtual Material* UseMaterial(MaterialData* pMaterialData) override;
		virtual void OnDrawMesh(Mesh* pMesh, uint32_t vertexOffset, uint32_t vertexCount) override;
		virtual void DrawScreenQuad() override;
		virtual void DispatchCompute(size_t num_groups_x, size_t num_groups_y, size_t num_groups_z) override;
		virtual void EnableDepthTest(bool enable) override;
		virtual void EnableDepthWrite(bool enable) override;
		virtual void SetViewport(int x, int y, uint32_t width, uint32_t height) override;
		virtual void Blit(RenderTexture* pTexture) override;

	protected:
		virtual void OnInitialize() override;
		virtual void OnCleanup() override;
		virtual GPUResourceManager* CreateGPUResourceManager() override;

	private:
		GLuint m_ScreenQuadVertexArrayID;
		GLuint m_ScreenQuadVertexbufferID;
	};
}
