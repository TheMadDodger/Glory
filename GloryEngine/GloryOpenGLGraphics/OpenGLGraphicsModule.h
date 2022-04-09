#pragma once
#include <GraphicsModule.h>
#include <GL/glew.h>
#include <ShaderData.h>
#include "GLMesh.h"

namespace Glory
{
	class OpenGLGraphicsModule : public GraphicsModule
	{
	public:
		OpenGLGraphicsModule();
		virtual ~OpenGLGraphicsModule();
		static void LogGLError(const GLenum& err, bool bIncludeTimeStamp = true);

	public: // Commands
		virtual void Clear(glm::vec4 color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)) override;
		virtual void Swap() override;
		virtual Material* UseMaterial(MaterialData* pMaterialData) override;
		virtual void OnDrawMesh(MeshData* pMeshData) override;
		virtual void DispatchCompute(size_t num_groups_x, size_t num_groups_y, size_t num_groups_z) override;

	protected:
		virtual void OnInitialize() override;
		virtual void OnCleanup() override;
		virtual void ThreadedCleanup() override;
		virtual void ThreadedInitialize() override;
		virtual GPUResourceManager* CreateGPUResourceManager() override;
	};
}
