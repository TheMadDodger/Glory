#pragma once
#include <GL/glew.h>
#include <GraphicsEnums.h>
#include <map>

namespace Glory
{
	class GLConverter
	{
	public:
		static GLuint GetGLImageType(const ImageType& imageType);
		static GLuint GetShaderStageFlag(const ShaderType& shaderType);

		static const std::map<PixelFormat, GLuint> TO_GLFORMAT;

	private:
		GLConverter() {}
		virtual ~GLConverter() {}
	};
}