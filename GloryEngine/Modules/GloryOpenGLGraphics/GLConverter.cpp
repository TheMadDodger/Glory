#include "GLConverter.h"
#include "GloryOGL.h"

namespace Glory
{
    const std::map<PixelFormat, GLuint> GLConverter::TO_GLFORMAT = {
        { PixelFormat::PF_Undefined,                 0 },

        // Basic format
        { PixelFormat::PF_R,                         GL_RED },
        { PixelFormat::PF_RG,                        GL_RG },
        { PixelFormat::PF_RGB,                       GL_RGB },
        { PixelFormat::PF_BGR,                       GL_BGR },
        { PixelFormat::PF_RGBA,                      GL_RGBA },
        { PixelFormat::PF_BGRA,                      GL_BGRA },
        { PixelFormat::PF_RI,                        GL_RED_INTEGER },
        { PixelFormat::PF_RGI,                       GL_RG_INTEGER },
        { PixelFormat::PF_RGBI,                      GL_RGB_INTEGER },
        { PixelFormat::PF_BGRI,                      GL_BGR_INTEGER },
        { PixelFormat::PF_RGBAI,                     GL_RGBA_INTEGER },
        { PixelFormat::PF_BGRAI,                     GL_BGRA_INTEGER },
        { PixelFormat::PF_Stencil,                   GL_STENCIL_INDEX },
        { PixelFormat::PF_Depth,                     GL_DEPTH_COMPONENT },
        { PixelFormat::PF_DepthStencil,              GL_DEPTH_STENCIL },

        // Internal format
        { PixelFormat::PF_R4G4UnormPack8,            0 },
        { PixelFormat::PF_R4G4B4A4UnormPack16,       0 },
        { PixelFormat::PF_B4G4R4A4UnormPack16,       0 },
        { PixelFormat::PF_R5G6B5UnormPack16,         0 },
        { PixelFormat::PF_B5G6R5UnormPack16,         0 },
        { PixelFormat::PF_R5G5B5A1UnormPack16,       0 },
        { PixelFormat::PF_B5G5R5A1UnormPack16,       0 },
        { PixelFormat::PF_A1R5G5B5UnormPack16,       0 },
        { PixelFormat::PF_R8Unorm,                   0 },
        { PixelFormat::PF_R8Snorm,                   0 },
        { PixelFormat::PF_R8Uscaled,                 0 },
        { PixelFormat::PF_R8Sscaled,                 0 },
        { PixelFormat::PF_R8Uint,                    GL_R8UI },
        { PixelFormat::PF_R8Sint,                    GL_R8I },
        { PixelFormat::PF_R8Srgb,                    GL_R8 },
        { PixelFormat::PF_R8G8Unorm,                 0 },
        { PixelFormat::PF_R8G8Snorm,                 0 },
        { PixelFormat::PF_R8G8Uscaled,               0 },
        { PixelFormat::PF_R8G8Sscaled,               0 },
        { PixelFormat::PF_R8G8Uint,                  0 },
        { PixelFormat::PF_R8G8Sint,                  0 },
        { PixelFormat::PF_R8G8Srgb,                  GL_RG8 },
        { PixelFormat::PF_R8G8B8Unorm,               0 },
        { PixelFormat::PF_R8G8B8Snorm,               0 },
        { PixelFormat::PF_R8G8B8Uscaled,             0 },
        { PixelFormat::PF_R8G8B8Sscaled,             0 },
        { PixelFormat::PF_R8G8B8Uint,                0 },
        { PixelFormat::PF_R8G8B8Sint,                0 },
        { PixelFormat::PF_R8G8B8Srgb,                GL_RGB8 },
        { PixelFormat::PF_B8G8R8Unorm,               0 },
        { PixelFormat::PF_B8G8R8Snorm,               0 },
        { PixelFormat::PF_B8G8R8Uscaled,             0 },
        { PixelFormat::PF_B8G8R8Sscaled,             0 },
        { PixelFormat::PF_B8G8R8Uint,                0 },
        { PixelFormat::PF_B8G8R8Sint,                0 },
        { PixelFormat::PF_B8G8R8Srgb,                0 },
        { PixelFormat::PF_R8G8B8A8Unorm,             0 },
        { PixelFormat::PF_R8G8B8A8Snorm,             0 },
        { PixelFormat::PF_R8G8B8A8Uscaled,           0 },
        { PixelFormat::PF_R8G8B8A8Sscaled,           0 },
        { PixelFormat::PF_R8G8B8A8Uint,              0 },
        { PixelFormat::PF_R8G8B8A8Sint,              0 },
        { PixelFormat::PF_R8G8B8A8Srgb,              GL_RGBA8 },
        { PixelFormat::PF_B8G8R8A8Unorm,             0 },
        { PixelFormat::PF_B8G8R8A8Snorm,             0 },
        { PixelFormat::PF_B8G8R8A8Uscaled,           0 },
        { PixelFormat::PF_B8G8R8A8Sscaled,           0 },
        { PixelFormat::PF_B8G8R8A8Uint,              0 },
        { PixelFormat::PF_B8G8R8A8Sint,              0 },
        { PixelFormat::PF_B8G8R8A8Srgb,              0 },
        { PixelFormat::PF_A8B8G8R8UnormPack32,       0 },
        { PixelFormat::PF_A8B8G8R8SnormPack32,       0 },
        { PixelFormat::PF_A8B8G8R8UscaledPack32,     0 },
        { PixelFormat::PF_A8B8G8R8SscaledPack32,     0 },
        { PixelFormat::PF_A8B8G8R8UintPack32,        0 },
        { PixelFormat::PF_A8B8G8R8SintPack32,        0 },
        { PixelFormat::PF_A8B8G8R8SrgbPack32,        0 },
        { PixelFormat::PF_A2R10G10B10UnormPack32,    0 },
        { PixelFormat::PF_A2R10G10B10SnormPack32,    0 },
        { PixelFormat::PF_A2R10G10B10UscaledPack32,  0 },
        { PixelFormat::PF_A2R10G10B10SscaledPack32,  0 },
        { PixelFormat::PF_A2R10G10B10UintPack32,     0 },
        { PixelFormat::PF_A2R10G10B10SintPack32,     0 },
        { PixelFormat::PF_A2B10G10R10UnormPack32,    0 },
        { PixelFormat::PF_A2B10G10R10SnormPack32,    0 },
        { PixelFormat::PF_A2B10G10R10UscaledPack32,  0 },
        { PixelFormat::PF_A2B10G10R10SscaledPack32,  0 },
        { PixelFormat::PF_A2B10G10R10UintPack32,     0 },
        { PixelFormat::PF_A2B10G10R10SintPack32,     0 },
        { PixelFormat::PF_R16Unorm,                  0 },
        { PixelFormat::PF_R16Snorm,                  0 },
        { PixelFormat::PF_R16Uscaled,                0 },
        { PixelFormat::PF_R16Sscaled,                0 },
        { PixelFormat::PF_R16Uint,                   GL_R16UI },
        { PixelFormat::PF_R16Sint,                   GL_R16I },
        { PixelFormat::PF_R16Sfloat,                 0 },
        { PixelFormat::PF_R16G16Unorm,               0 },
        { PixelFormat::PF_R16G16Snorm,               0 },
        { PixelFormat::PF_R16G16Uscaled,             0 },
        { PixelFormat::PF_R16G16Sscaled,             0 },
        { PixelFormat::PF_R16G16Uint,                0 },
        { PixelFormat::PF_R16G16Sint,                0 },
        { PixelFormat::PF_R16G16Sfloat,              0 },
        { PixelFormat::PF_R16G16B16Unorm,            0 },
        { PixelFormat::PF_R16G16B16Snorm,            0 },
        { PixelFormat::PF_R16G16B16Uscaled,          0 },
        { PixelFormat::PF_R16G16B16Sscaled,          0 },
        { PixelFormat::PF_R16G16B16Uint,             0 },
        { PixelFormat::PF_R16G16B16Sint,             0 },
        { PixelFormat::PF_R16G16B16Sfloat,           0 },
        { PixelFormat::PF_R16G16B16A16Unorm,         0 },
        { PixelFormat::PF_R16G16B16A16Snorm ,        0 },
        { PixelFormat::PF_R16G16B16A16Uscaled,       0 },
        { PixelFormat::PF_R16G16B16A16Sscaled,       0 },
        { PixelFormat::PF_R16G16B16A16Uint,          0 },
        { PixelFormat::PF_R16G16B16A16Sint,          0 },
        { PixelFormat::PF_R16G16B16A16Sfloat,        0 },
        { PixelFormat::PF_R32Uint,                   GL_R32UI },
        { PixelFormat::PF_R32Sint,                   GL_R32I },
        { PixelFormat::PF_R32Sfloat,                 0 },
        { PixelFormat::PF_R32G32Uint,                0 },
        { PixelFormat::PF_R32G32Sint,                0 },
        { PixelFormat::PF_R32G32Sfloat,              0 },
        { PixelFormat::PF_R32G32B32Uint,             0 },
        { PixelFormat::PF_R32G32B32Sint,             0 },
        { PixelFormat::PF_R32G32B32Sfloat,           0 },
        { PixelFormat::PF_R32G32B32A32Uint,          0 },
        { PixelFormat::PF_R32G32B32A32Sint,          0 },
        { PixelFormat::PF_R32G32B32A32Sfloat,        0 },
        { PixelFormat::PF_R64Uint,                   0 }, // Not supported
        { PixelFormat::PF_R64Sint,                   0 }, // Not supported
        { PixelFormat::PF_R64Sfloat,                 0 },
        { PixelFormat::PF_R64G64Uint,                0 },
        { PixelFormat::PF_R64G64Sint,                0 },
        { PixelFormat::PF_R64G64Sfloat,              0 },
        { PixelFormat::PF_R64G64B64Uint,             0 },
        { PixelFormat::PF_R64G64B64Sint,             0 },
        { PixelFormat::PF_R64G64B64Sfloat,           0 },
        { PixelFormat::PF_R64G64B64A64Uint,          0 },
        { PixelFormat::PF_R64G64B64A64Sint,          0 },
        { PixelFormat::PF_R64G64B64A64Sfloat,        0 },
        { PixelFormat::PF_B10G11R11UfloatPack32,     0 },
        { PixelFormat::PF_E5B9G9R9UfloatPack32,      0 },
        { PixelFormat::PF_D16Unorm,                  0 },
        { PixelFormat::PF_X8D24UnormPack32,          0 },
        { PixelFormat::PF_D32Sfloat,                 0 },
        { PixelFormat::PF_S8Uint,                    0 },
        { PixelFormat::PF_D16UnormS8Uint,            0 },
        { PixelFormat::PF_D24UnormS8Uint,            0 },
        { PixelFormat::PF_D32SfloatS8Uint,           0 },
        { PixelFormat::PF_Bc1RgbUnormBlock,          0 },
        { PixelFormat::PF_Bc1RgbSrgbBlock,           0 },
        { PixelFormat::PF_Bc1RgbaUnormBlock,         0 },
        { PixelFormat::PF_Bc1RgbaSrgbBlock,          0 },
        { PixelFormat::PF_Bc2UnormBlock,             0 },
        { PixelFormat::PF_Bc2SrgbBlock,              0 },
        { PixelFormat::PF_Bc3UnormBlock,             0 },
        { PixelFormat::PF_Bc3SrgbBlock,              0 },
        { PixelFormat::PF_Bc4UnormBlock,             0 },
        { PixelFormat::PF_Bc4SnormBlock,             0 },
        { PixelFormat::PF_Bc5UnormBlock,             0 },
        { PixelFormat::PF_Bc5SnormBlock,             0 },
        { PixelFormat::PF_Bc6HUfloatBlock,           0 },
        { PixelFormat::PF_Bc6HSfloatBlock,           0 },
        { PixelFormat::PF_Bc7UnormBlock,             0 },
        { PixelFormat::PF_Bc7SrgbBlock,              0 },
        { PixelFormat::PF_Etc2R8G8B8UnormBlock,      0 },
        { PixelFormat::PF_Etc2R8G8B8SrgbBlock,       0 },
        { PixelFormat::PF_Etc2R8G8B8A1UnormBlock,    0 },
        { PixelFormat::PF_Etc2R8G8B8A1SrgbBlock,     0 },
        { PixelFormat::PF_Etc2R8G8B8A8UnormBlock,    0 },
        { PixelFormat::PF_Etc2R8G8B8A8SrgbBlock,     0 },
        { PixelFormat::PF_EacR11UnormBlock,          0 },
        { PixelFormat::PF_EacR11SnormBlock,          0 },
        { PixelFormat::PF_EacR11G11UnormBlock,       0 },
        { PixelFormat::PF_EacR11G11SnormBlock,       0 },
        { PixelFormat::PF_Astc4x4UnormBlock,         0 },
        { PixelFormat::PF_Astc4x4SrgbBlock,          GL_COMPRESSED_RGBA_ASTC_4x4_KHR },
        { PixelFormat::PF_Astc5x4UnormBlock,         0 },
        { PixelFormat::PF_Astc5x4SrgbBlock,          GL_COMPRESSED_RGBA_ASTC_5x4_KHR },
        { PixelFormat::PF_Astc5x5UnormBlock,         0 },
        { PixelFormat::PF_Astc5x5SrgbBlock,          GL_COMPRESSED_RGBA_ASTC_5x5_KHR },
        { PixelFormat::PF_Astc6x5UnormBlock,         0 },
        { PixelFormat::PF_Astc6x5SrgbBlock,          0 },
        { PixelFormat::PF_Astc6x6UnormBlock,         0 },
        { PixelFormat::PF_Astc6x6SrgbBlock,          0 },
        { PixelFormat::PF_Astc8x5UnormBlock,         0 },
        { PixelFormat::PF_Astc8x5SrgbBlock,          0 },
        { PixelFormat::PF_Astc8x6UnormBlock,         0 },
        { PixelFormat::PF_Astc8x6SrgbBlock,          0 },
        { PixelFormat::PF_Astc8x8UnormBlock,         0 },
        { PixelFormat::PF_Astc8x8SrgbBlock,          0 },
        { PixelFormat::PF_Astc10x5UnormBlock,        0 },
        { PixelFormat::PF_Astc10x5SrgbBlock,         0 },
        { PixelFormat::PF_Astc10x6UnormBlock,        0 },
        { PixelFormat::PF_Astc10x6SrgbBlock,         0 },
        { PixelFormat::PF_Astc10x8UnormBlock,        0 },
        { PixelFormat::PF_Astc10x8SrgbBlock,         0 },
        { PixelFormat::PF_Astc10x10UnormBlock,       0 },
        { PixelFormat::PF_Astc10x10SrgbBlock,        GL_COMPRESSED_RGBA_ASTC_10x10_KHR },
        { PixelFormat::PF_Astc12x10UnormBlock,       0 },
        { PixelFormat::PF_Astc12x10SrgbBlock,        0 },
        { PixelFormat::PF_Astc12x12UnormBlock,       0 },
        { PixelFormat::PF_Astc12x12SrgbBlock,        0 },
        { PixelFormat::PF_Depth16,                   GL_DEPTH_COMPONENT16 },
        { PixelFormat::PF_Depth24,                   GL_DEPTH_COMPONENT24 },
        { PixelFormat::PF_Depth32,                   GL_DEPTH_COMPONENT32 },
    };

    const std::map<BufferBindingTarget, GLuint> GLConverter::TO_GLBUFFERTARGET
    {
        { BufferBindingTarget::B_ARRAY,               GL_ARRAY_BUFFER              },
        { BufferBindingTarget::B_ATOMIC_COUNTER,      GL_ATOMIC_COUNTER_BUFFER     },
        { BufferBindingTarget::B_COPY_READ,           GL_COPY_READ_BUFFER          },
        { BufferBindingTarget::B_COPY_WRITE,          GL_COPY_WRITE_BUFFER         },
        { BufferBindingTarget::B_DISPATCH_INDIRECT,   GL_DISPATCH_INDIRECT_BUFFER  },
        { BufferBindingTarget::B_DRAW_INDIRECT,       GL_DRAW_INDIRECT_BUFFER      },
        { BufferBindingTarget::B_ELEMENT_ARRAY,       GL_ELEMENT_ARRAY_BUFFER      },
        { BufferBindingTarget::B_PIXEL_PACK,          GL_PIXEL_PACK_BUFFER         },
        { BufferBindingTarget::B_PIXEL_UNPACK,        GL_PIXEL_UNPACK_BUFFER       },
        { BufferBindingTarget::B_QUERY,               GL_QUERY_BUFFER              },
        { BufferBindingTarget::B_SHADER_STORAGE,      GL_SHADER_STORAGE_BUFFER     },
        { BufferBindingTarget::B_TEXTURE,             GL_TEXTURE_BUFFER            },
        { BufferBindingTarget::B_TRANSFORM_FEEDBACK,  GL_TRANSFORM_FEEDBACK_BUFFER },
        { BufferBindingTarget::B_UNIFORM,             GL_UNIFORM_BUFFER            },
    };

    const std::map<MemoryUsage, GLuint> GLConverter::TO_GLBUFFERUSAGE =
    {
        { MemoryUsage::MU_STREAM_DRAW,  GL_STREAM_DRAW  },
        { MemoryUsage::MU_STREAM_READ,  GL_STREAM_READ  },
        { MemoryUsage::MU_STREAM_COPY,  GL_STREAM_COPY  },
        { MemoryUsage::MU_STATIC_DRAW,  GL_STATIC_DRAW  },
        { MemoryUsage::MU_STATIC_READ,  GL_STATIC_READ  },
        { MemoryUsage::MU_STATIC_COPY,  GL_STATIC_COPY  },
        { MemoryUsage::MU_DYNAMIC_DRAW, GL_DYNAMIC_DRAW },
        { MemoryUsage::MU_DYNAMIC_READ, GL_DYNAMIC_READ },
        { MemoryUsage::MU_DYNAMIC_COPY, GL_DYNAMIC_COPY },
    };

    const std::map<PrimitiveType, GLuint> GLConverter::TO_GLPRIMITIVETYPE =
    {
        { PrimitiveType::PT_Point, GL_POINT  },
        { PrimitiveType::PT_LineStrip, GL_LINE_STRIP  },
        { PrimitiveType::PT_LineLoop, GL_LINE_LOOP  },
        { PrimitiveType::PT_Lines, GL_LINES  },
        { PrimitiveType::PT_LineStripAdjacency, GL_LINE_STRIP_ADJACENCY  },
        { PrimitiveType::PT_LinesAdjacency, GL_LINES_ADJACENCY  },
        { PrimitiveType::PT_TriangleStrip, GL_TRIANGLE_STRIP },
        { PrimitiveType::PT_TriangleFan, GL_TRIANGLE_FAN },
        { PrimitiveType::PT_Triangles, GL_TRIANGLES },
        { PrimitiveType::PT_TriangleStripAdjacency, GL_TRIANGLE_STRIP_ADJACENCY },
        { PrimitiveType::PT_TrianglesAdjacency, GL_TRIANGLES_ADJACENCY },
        { PrimitiveType::PT_Patches, GL_PATCHES },
    };

    const std::map<Filter, GLint> GLConverter::TO_GLFILTER = {
        { Filter::F_Linear, GL_LINEAR },
        { Filter::F_Nearest, GL_NEAREST },
        { Filter::F_CubicEXT, GL_CUBIC_EXT },
        { Filter::F_CubicIMG, GL_CUBIC_IMG },
    };

    const std::map<SamplerAddressMode, GLint> GLConverter::TO_GLTEXTUREWRAP = {
        { SamplerAddressMode::SAM_Repeat, GL_REPEAT },
        { SamplerAddressMode::SAM_ClampToEdge, GL_CLAMP_TO_EDGE },
        { SamplerAddressMode::SAM_ClampToBorder, GL_CLAMP_TO_BORDER },
        { SamplerAddressMode::SAM_MirroredRepeat, GL_MIRRORED_REPEAT },
        { SamplerAddressMode::SAM_MirrorClampToEdge, GL_MIRROR_CLAMP_TO_EDGE },
        { SamplerAddressMode::SAM_MirrorClampToEdgeKHR, GL_MIRROR_CLAMP_TO_EDGE_EXT },
    };

    GLuint GLConverter::GetGLImageType(const ImageType& imageType)
    {
        switch (imageType)
        {
        case Glory::ImageType::IT_UNDEFINED:
            break;
        case Glory::ImageType::IT_1D:
            return GL_TEXTURE_1D;
        case Glory::ImageType::IT_2D:
            return GL_TEXTURE_2D;
        case Glory::ImageType::IT_3D:
            return GL_TEXTURE_3D;
        case Glory::ImageType::IT_Cube:
            return GL_TEXTURE_CUBE_MAP;
        case Glory::ImageType::IT_1DArray:
            return GL_TEXTURE_1D_ARRAY;
        case Glory::ImageType::IT_2DArray:
            return GL_TEXTURE_2D_ARRAY;
        case Glory::ImageType::IT_CubeArray:
            return GL_TEXTURE_CUBE_MAP_ARRAY;
        default:
            break;
        }

        return 0;
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