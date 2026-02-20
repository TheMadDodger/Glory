#pragma once
#include "OpenGLDevice.h"

#include <Module.h>
#include <GL/glew.h>

namespace Glory
{
	class OpenGLGraphicsModule : public Module
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
		/* OpenGL can only have 1 device */
		OpenGLDevice m_Device;
	};
}
