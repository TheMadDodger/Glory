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
		static const std::map<BufferBindingTarget, GLuint> TO_GLBUFFERTARGET;
		static const std::map<MemoryUsage, GLuint> TO_GLBUFFERUSAGE;

	private:
		GLConverter() {}
		virtual ~GLConverter() {}
	};
}