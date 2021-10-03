#include "GLConverter.h"

namespace Glory
{
    GLuint GLConverter::GetGLImageViewType(const ImageType& imageType)
    {
        return GLuint();
    }

    GLuint GLConverter::GetGLImageType(const ImageType& imageType)
    {
        return GLuint();
    }
    GLuint GLConverter::GetGLImageAspectFlags(const ImageAspect& aspectFlags)
    {
        return GLuint();
    }
    GLuint GLConverter::GetGLFilter(const Filter& filter)
    {
        return GLuint();
    }
    GLuint GLConverter::GetGLCompareOp(const CompareOp& op)
    {
        return GLuint();
    }
    GLuint GLConverter::GetGLSamplerMipmapMode(const Filter& filter)
    {
        return GLuint();
    }
    GLuint GLConverter::GetSamplerAddressMode(const SamplerAddressMode& mode)
    {
        return GLuint();
    }
    GLuint GLConverter::GetGLSamplerInfo(const SamplerSettings& settings)
    {
        return GLuint();
    }
    GLuint GLConverter::GetGLFormat(const PixelFormat& format)
    {
        return GLuint();
    }
    GLuint GLConverter::GetShaderStageFlag(const ShaderType& shaderType)
    {
        switch (shaderType)
        {
        case ShaderType::ST_Vertex:
            return GL_VERTEX_SHADER;
        case ShaderType::ST_Fragment:
            return GL_FRAGMENT_SHADER;
        case ShaderType::ST_Geomtery:
            return GL_GEOMETRY_SHADER;
        case ShaderType::ST_TessControl:
            return GL_TESS_CONTROL_SHADER;
        case ShaderType::ST_TessEval:
            return GL_TESS_EVALUATION_SHADER;
        case ShaderType::ST_Compute:
            return GL_COMPUTE_SHADER;
        }

        return 0;
    }
}