#pragma once
#include <GL/glew.h>
#include <GraphicsEnums.h>

namespace Glory
{
	class GLConverter
	{
	public:
		static GLuint GetGLImageViewType(const ImageType& imageType);
		static GLuint GetGLImageType(const ImageType& imageType);
		static GLuint GetGLImageAspectFlags(const ImageAspect& aspectFlags);
		static GLuint GetGLFilter(const Filter& filter);
		static GLuint GetGLCompareOp(const CompareOp& op);
		static GLuint GetGLSamplerMipmapMode(const Filter& filter);
		static GLuint GetSamplerAddressMode(const SamplerAddressMode& mode);
		static GLuint GetGLSamplerInfo(const SamplerSettings& settings);
		static GLuint GetGLFormat(const PixelFormat& format);
		static GLuint GetShaderStageFlag(const ShaderType& shaderType);

	private:
		GLConverter() {}
		virtual ~GLConverter() {}
	};
}