#pragma once
#include "OpenGLDevice.h"

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

		/** @brief OpenGLGraphicsModule type */
		const std::type_info& GetModuleType() override;

		GLORY_MODULE_VERSION_H(0, 3, 0);

	protected:
		virtual void PreInitialize() override;
		virtual void Initialize() override;
		virtual void Cleanup() override;

	private:
		GLuint m_ScreenQuadVertexArrayID;
		GLuint m_ScreenQuadVertexbufferID;
		GLuint m_UnitCubeVertexArrayID;
		GLuint m_UnitCubeVertexbufferID;

		MaterialData* m_pPassthroughMaterial = nullptr;

		/* OpenGL can only have 1 device */
		OpenGLDevice m_Device;
	};
}
