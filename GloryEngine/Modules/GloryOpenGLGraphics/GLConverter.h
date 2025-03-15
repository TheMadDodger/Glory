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
		static GLint GetMinFilter(Filter mipMap, Filter minFilter);

		static const std::map<PixelFormat, GLuint> TO_GLFORMAT;
		static const std::map<BufferBindingTarget, GLuint> TO_GLBUFFERTARGET;
		static const std::map<MemoryUsage, GLuint> TO_GLBUFFERUSAGE;
		static const std::map<PrimitiveType, GLuint> TO_GLPRIMITIVETYPE;
		static const std::map<Filter, GLint> TO_GLFILTER;
		static const std::map<SamplerAddressMode, GLint> TO_GLTEXTUREWRAP;
		static const std::map<DataType, GLenum> TO_GLDATATYPE;
		static const std::map<CompareOp, GLenum> TO_GLOP;
		static const std::map<Func, GLenum> TO_GLFUNC;

	private:
		GLConverter() {}
		virtual ~GLConverter() {}
	};
}