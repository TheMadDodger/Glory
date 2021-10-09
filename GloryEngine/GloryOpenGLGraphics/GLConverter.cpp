#include "GLConverter.h"

namespace Glory
{
    GLuint GLConverter::GetGLImageViewType(const ImageType& imageType)
    {
        return GLuint();
    }

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
        switch (format)
        {
        case Glory::PixelFormat::PF_Undefined:
            return 0;
        case Glory::PixelFormat::PF_R4G4UnormPack8:
            break;
        case Glory::PixelFormat::PF_R4G4B4A4UnormPack16:
            break;
        case Glory::PixelFormat::PF_B4G4R4A4UnormPack16:
            break;
        case Glory::PixelFormat::PF_R5G6B5UnormPack16:
            break;
        case Glory::PixelFormat::PF_B5G6R5UnormPack16:
            break;
        case Glory::PixelFormat::PF_R5G5B5A1UnormPack16:
            break;
        case Glory::PixelFormat::PF_B5G5R5A1UnormPack16:
            break;
        case Glory::PixelFormat::PF_A1R5G5B5UnormPack16:
            break;
        case Glory::PixelFormat::PF_R8Unorm:
            break;
        case Glory::PixelFormat::PF_R8Snorm:
            break;
        case Glory::PixelFormat::PF_R8Uscaled:
            break;
        case Glory::PixelFormat::PF_R8Sscaled:
            break;
        case Glory::PixelFormat::PF_R8Uint:
            break;
        case Glory::PixelFormat::PF_R8Sint:
            break;
        case Glory::PixelFormat::PF_R8Srgb:
            return GL_R;
        case Glory::PixelFormat::PF_R8G8Unorm:
            break;
        case Glory::PixelFormat::PF_R8G8Snorm:
            break;
        case Glory::PixelFormat::PF_R8G8Uscaled:
            break;
        case Glory::PixelFormat::PF_R8G8Sscaled:
            break;
        case Glory::PixelFormat::PF_R8G8Uint:
            break;
        case Glory::PixelFormat::PF_R8G8Sint:
            break;
        case Glory::PixelFormat::PF_R8G8Srgb:
            return GL_RG;
            break;
        case Glory::PixelFormat::PF_R8G8B8Unorm:
            break;
        case Glory::PixelFormat::PF_R8G8B8Snorm:
            break;
        case Glory::PixelFormat::PF_R8G8B8Uscaled:
            break;
        case Glory::PixelFormat::PF_R8G8B8Sscaled:
            break;
        case Glory::PixelFormat::PF_R8G8B8Uint:
            break;
        case Glory::PixelFormat::PF_R8G8B8Sint:
            break;
        case Glory::PixelFormat::PF_R8G8B8Srgb:
            return GL_RGB;
            break;
        case Glory::PixelFormat::PF_B8G8R8Unorm:
            break;
        case Glory::PixelFormat::PF_B8G8R8Snorm:
            break;
        case Glory::PixelFormat::PF_B8G8R8Uscaled:
            break;
        case Glory::PixelFormat::PF_B8G8R8Sscaled:
            break;
        case Glory::PixelFormat::PF_B8G8R8Uint:
            break;
        case Glory::PixelFormat::PF_B8G8R8Sint:
            break;
        case Glory::PixelFormat::PF_B8G8R8Srgb:
            return GL_BGR;
            break;
        case Glory::PixelFormat::PF_R8G8B8A8Unorm:
            break;
        case Glory::PixelFormat::PF_R8G8B8A8Snorm:
            break;
        case Glory::PixelFormat::PF_R8G8B8A8Uscaled:
            break;
        case Glory::PixelFormat::PF_R8G8B8A8Sscaled:
            break;
        case Glory::PixelFormat::PF_R8G8B8A8Uint:
            break;
        case Glory::PixelFormat::PF_R8G8B8A8Sint:
            break;
        case Glory::PixelFormat::PF_R8G8B8A8Srgb:
            return GL_RGBA;
        case Glory::PixelFormat::PF_B8G8R8A8Unorm:
            break;
        case Glory::PixelFormat::PF_B8G8R8A8Snorm:
            break;
        case Glory::PixelFormat::PF_B8G8R8A8Uscaled:
            break;
        case Glory::PixelFormat::PF_B8G8R8A8Sscaled:
            break;
        case Glory::PixelFormat::PF_B8G8R8A8Uint:
            break;
        case Glory::PixelFormat::PF_B8G8R8A8Sint:
            break;
        case Glory::PixelFormat::PF_B8G8R8A8Srgb:
            return GL_BGRA;
        case Glory::PixelFormat::PF_A8B8G8R8UnormPack32:
            break;
        case Glory::PixelFormat::PF_A8B8G8R8SnormPack32:
            break;
        case Glory::PixelFormat::PF_A8B8G8R8UscaledPack32:
            break;
        case Glory::PixelFormat::PF_A8B8G8R8SscaledPack32:
            break;
        case Glory::PixelFormat::PF_A8B8G8R8UintPack32:
            break;
        case Glory::PixelFormat::PF_A8B8G8R8SintPack32:
            break;
        case Glory::PixelFormat::PF_A8B8G8R8SrgbPack32:
            break;
        case Glory::PixelFormat::PF_A2R10G10B10UnormPack32:
            break;
        case Glory::PixelFormat::PF_A2R10G10B10SnormPack32:
            break;
        case Glory::PixelFormat::PF_A2R10G10B10UscaledPack32:
            break;
        case Glory::PixelFormat::PF_A2R10G10B10SscaledPack32:
            break;
        case Glory::PixelFormat::PF_A2R10G10B10UintPack32:
            break;
        case Glory::PixelFormat::PF_A2R10G10B10SintPack32:
            break;
        case Glory::PixelFormat::PF_A2B10G10R10UnormPack32:
            break;
        case Glory::PixelFormat::PF_A2B10G10R10SnormPack32:
            break;
        case Glory::PixelFormat::PF_A2B10G10R10UscaledPack32:
            break;
        case Glory::PixelFormat::PF_A2B10G10R10SscaledPack32:
            break;
        case Glory::PixelFormat::PF_A2B10G10R10UintPack32:
            break;
        case Glory::PixelFormat::PF_A2B10G10R10SintPack32:
            break;
        case Glory::PixelFormat::PF_R16Unorm:
            break;
        case Glory::PixelFormat::PF_R16Snorm:
            break;
        case Glory::PixelFormat::PF_R16Uscaled:
            break;
        case Glory::PixelFormat::PF_R16Sscaled:
            break;
        case Glory::PixelFormat::PF_R16Uint:
            break;
        case Glory::PixelFormat::PF_R16Sint:
            break;
        case Glory::PixelFormat::PF_R16Sfloat:
            return GL_R16;
            break;
        case Glory::PixelFormat::PF_R16G16Unorm:
            break;
        case Glory::PixelFormat::PF_R16G16Snorm:
            break;
        case Glory::PixelFormat::PF_R16G16Uscaled:
            break;
        case Glory::PixelFormat::PF_R16G16Sscaled:
            break;
        case Glory::PixelFormat::PF_R16G16Uint:
            break;
        case Glory::PixelFormat::PF_R16G16Sint:
            break;
        case Glory::PixelFormat::PF_R16G16Sfloat:
            break;
        case Glory::PixelFormat::PF_R16G16B16Unorm:
            break;
        case Glory::PixelFormat::PF_R16G16B16Snorm:
            break;
        case Glory::PixelFormat::PF_R16G16B16Uscaled:
            break;
        case Glory::PixelFormat::PF_R16G16B16Sscaled:
            break;
        case Glory::PixelFormat::PF_R16G16B16Uint:
            break;
        case Glory::PixelFormat::PF_R16G16B16Sint:
            break;
        case Glory::PixelFormat::PF_R16G16B16Sfloat:
            break;
        case Glory::PixelFormat::PF_R16G16B16A16Unorm:
            break;
        case Glory::PixelFormat::PF_R16G16B16A16Snorm:
            break;
        case Glory::PixelFormat::PF_R16G16B16A16Uscaled:
            break;
        case Glory::PixelFormat::PF_R16G16B16A16Sscaled:
            break;
        case Glory::PixelFormat::PF_R16G16B16A16Uint:
            break;
        case Glory::PixelFormat::PF_R16G16B16A16Sint:
            break;
        case Glory::PixelFormat::PF_R16G16B16A16Sfloat:
            break;
        case Glory::PixelFormat::PF_R32Uint:
            break;
        case Glory::PixelFormat::PF_R32Sint:
            break;
        case Glory::PixelFormat::PF_R32Sfloat:
            break;
        case Glory::PixelFormat::PF_R32G32Uint:
            break;
        case Glory::PixelFormat::PF_R32G32Sint:
            break;
        case Glory::PixelFormat::PF_R32G32Sfloat:
            break;
        case Glory::PixelFormat::PF_R32G32B32Uint:
            break;
        case Glory::PixelFormat::PF_R32G32B32Sint:
            break;
        case Glory::PixelFormat::PF_R32G32B32Sfloat:
            break;
        case Glory::PixelFormat::PF_R32G32B32A32Uint:
            break;
        case Glory::PixelFormat::PF_R32G32B32A32Sint:
            break;
        case Glory::PixelFormat::PF_R32G32B32A32Sfloat:
            break;
        case Glory::PixelFormat::PF_R64Uint:
            break;
        case Glory::PixelFormat::PF_R64Sint:
            break;
        case Glory::PixelFormat::PF_R64Sfloat:
            break;
        case Glory::PixelFormat::PF_R64G64Uint:
            break;
        case Glory::PixelFormat::PF_R64G64Sint:
            break;
        case Glory::PixelFormat::PF_R64G64Sfloat:
            break;
        case Glory::PixelFormat::PF_R64G64B64Uint:
            break;
        case Glory::PixelFormat::PF_R64G64B64Sint:
            break;
        case Glory::PixelFormat::PF_R64G64B64Sfloat:
            break;
        case Glory::PixelFormat::PF_R64G64B64A64Uint:
            break;
        case Glory::PixelFormat::PF_R64G64B64A64Sint:
            break;
        case Glory::PixelFormat::PF_R64G64B64A64Sfloat:
            break;
        case Glory::PixelFormat::PF_B10G11R11UfloatPack32:
            break;
        case Glory::PixelFormat::PF_E5B9G9R9UfloatPack32:
            break;
        case Glory::PixelFormat::PF_D16Unorm:
            break;
        case Glory::PixelFormat::PF_X8D24UnormPack32:
            break;
        case Glory::PixelFormat::PF_D32Sfloat:
            break;
        case Glory::PixelFormat::PF_S8Uint:
            break;
        case Glory::PixelFormat::PF_D16UnormS8Uint:
            break;
        case Glory::PixelFormat::PF_D24UnormS8Uint:
            break;
        case Glory::PixelFormat::PF_D32SfloatS8Uint:
            break;
        case Glory::PixelFormat::PF_Bc1RgbUnormBlock:
            break;
        case Glory::PixelFormat::PF_Bc1RgbSrgbBlock:
            break;
        case Glory::PixelFormat::PF_Bc1RgbaUnormBlock:
            break;
        case Glory::PixelFormat::PF_Bc1RgbaSrgbBlock:
            break;
        case Glory::PixelFormat::PF_Bc2UnormBlock:
            break;
        case Glory::PixelFormat::PF_Bc2SrgbBlock:
            break;
        case Glory::PixelFormat::PF_Bc3UnormBlock:
            break;
        case Glory::PixelFormat::PF_Bc3SrgbBlock:
            break;
        case Glory::PixelFormat::PF_Bc4UnormBlock:
            break;
        case Glory::PixelFormat::PF_Bc4SnormBlock:
            break;
        case Glory::PixelFormat::PF_Bc5UnormBlock:
            break;
        case Glory::PixelFormat::PF_Bc5SnormBlock:
            break;
        case Glory::PixelFormat::PF_Bc6HUfloatBlock:
            break;
        case Glory::PixelFormat::PF_Bc6HSfloatBlock:
            break;
        case Glory::PixelFormat::PF_Bc7UnormBlock:
            break;
        case Glory::PixelFormat::PF_Bc7SrgbBlock:
            break;
        case Glory::PixelFormat::PF_Etc2R8G8B8UnormBlock:
            break;
        case Glory::PixelFormat::PF_Etc2R8G8B8SrgbBlock:
            break;
        case Glory::PixelFormat::PF_Etc2R8G8B8A1UnormBlock:
            break;
        case Glory::PixelFormat::PF_Etc2R8G8B8A1SrgbBlock:
            break;
        case Glory::PixelFormat::PF_Etc2R8G8B8A8UnormBlock:
            break;
        case Glory::PixelFormat::PF_Etc2R8G8B8A8SrgbBlock:
            break;
        case Glory::PixelFormat::PF_EacR11UnormBlock:
            break;
        case Glory::PixelFormat::PF_EacR11SnormBlock:
            break;
        case Glory::PixelFormat::PF_EacR11G11UnormBlock:
            break;
        case Glory::PixelFormat::PF_EacR11G11SnormBlock:
            break;
        case Glory::PixelFormat::PF_Astc4x4UnormBlock:
            break;
        case Glory::PixelFormat::PF_Astc4x4SrgbBlock:
            break;
        case Glory::PixelFormat::PF_Astc5x4UnormBlock:
            break;
        case Glory::PixelFormat::PF_Astc5x4SrgbBlock:
            break;
        case Glory::PixelFormat::PF_Astc5x5UnormBlock:
            break;
        case Glory::PixelFormat::PF_Astc5x5SrgbBlock:
            break;
        case Glory::PixelFormat::PF_Astc6x5UnormBlock:
            break;
        case Glory::PixelFormat::PF_Astc6x5SrgbBlock:
            break;
        case Glory::PixelFormat::PF_Astc6x6UnormBlock:
            break;
        case Glory::PixelFormat::PF_Astc6x6SrgbBlock:
            break;
        case Glory::PixelFormat::PF_Astc8x5UnormBlock:
            break;
        case Glory::PixelFormat::PF_Astc8x5SrgbBlock:
            break;
        case Glory::PixelFormat::PF_Astc8x6UnormBlock:
            break;
        case Glory::PixelFormat::PF_Astc8x6SrgbBlock:
            break;
        case Glory::PixelFormat::PF_Astc8x8UnormBlock:
            break;
        case Glory::PixelFormat::PF_Astc8x8SrgbBlock:
            break;
        case Glory::PixelFormat::PF_Astc10x5UnormBlock:
            break;
        case Glory::PixelFormat::PF_Astc10x5SrgbBlock:
            break;
        case Glory::PixelFormat::PF_Astc10x6UnormBlock:
            break;
        case Glory::PixelFormat::PF_Astc10x6SrgbBlock:
            break;
        case Glory::PixelFormat::PF_Astc10x8UnormBlock:
            break;
        case Glory::PixelFormat::PF_Astc10x8SrgbBlock:
            break;
        case Glory::PixelFormat::PF_Astc10x10UnormBlock:
            break;
        case Glory::PixelFormat::PF_Astc10x10SrgbBlock:
            break;
        case Glory::PixelFormat::PF_Astc12x10UnormBlock:
            break;
        case Glory::PixelFormat::PF_Astc12x10SrgbBlock:
            break;
        case Glory::PixelFormat::PF_Astc12x12UnormBlock:
            break;
        case Glory::PixelFormat::PF_Astc12x12SrgbBlock:
            break;
        case Glory::PixelFormat::PF_G8B8G8R8422Unorm:
            break;
        case Glory::PixelFormat::PF_B8G8R8G8422Unorm:
            break;
        case Glory::PixelFormat::PF_G8B8R83Plane420Unorm:
            break;
        case Glory::PixelFormat::PF_G8B8R82Plane420Unorm:
            break;
        case Glory::PixelFormat::PF_G8B8R83Plane422Unorm:
            break;
        case Glory::PixelFormat::PF_G8B8R82Plane422Unorm:
            break;
        case Glory::PixelFormat::PF_G8B8R83Plane444Unorm:
            break;
        case Glory::PixelFormat::PF_R10X6UnormPack16:
            break;
        case Glory::PixelFormat::PF_R10X6G10X6Unorm2Pack16:
            break;
        case Glory::PixelFormat::PF_R10X6G10X6B10X6A10X6Unorm4Pack16:
            break;
        case Glory::PixelFormat::PF_G10X6B10X6G10X6R10X6422Unorm4Pack16:
            break;
        case Glory::PixelFormat::PF_B10X6G10X6R10X6G10X6422Unorm4Pack16:
            break;
        case Glory::PixelFormat::PF_G10X6B10X6R10X63Plane420Unorm3Pack16:
            break;
        case Glory::PixelFormat::PF_G10X6B10X6R10X62Plane420Unorm3Pack16:
            break;
        case Glory::PixelFormat::PF_G10X6B10X6R10X63Plane422Unorm3Pack16:
            break;
        case Glory::PixelFormat::PF_G10X6B10X6R10X62Plane422Unorm3Pack16:
            break;
        case Glory::PixelFormat::PF_G10X6B10X6R10X63Plane444Unorm3Pack16:
            break;
        case Glory::PixelFormat::PF_R12X4UnormPack16:
            break;
        case Glory::PixelFormat::PF_R12X4G12X4Unorm2Pack16:
            break;
        case Glory::PixelFormat::PF_R12X4G12X4B12X4A12X4Unorm4Pack16:
            break;
        case Glory::PixelFormat::PF_G12X4B12X4G12X4R12X4422Unorm4Pack16:
            break;
        case Glory::PixelFormat::PF_B12X4G12X4R12X4G12X4422Unorm4Pack16:
            break;
        case Glory::PixelFormat::PF_G12X4B12X4R12X43Plane420Unorm3Pack16:
            break;
        case Glory::PixelFormat::PF_G12X4B12X4R12X42Plane420Unorm3Pack16:
            break;
        case Glory::PixelFormat::PF_G12X4B12X4R12X43Plane422Unorm3Pack16:
            break;
        case Glory::PixelFormat::PF_G12X4B12X4R12X42Plane422Unorm3Pack16:
            break;
        case Glory::PixelFormat::PF_G12X4B12X4R12X43Plane444Unorm3Pack16:
            break;
        case Glory::PixelFormat::PF_G16B16G16R16422Unorm:
            break;
        case Glory::PixelFormat::PF_B16G16R16G16422Unorm:
            break;
        case Glory::PixelFormat::PF_G16B16R163Plane420Unorm:
            break;
        case Glory::PixelFormat::PF_G16B16R162Plane420Unorm:
            break;
        case Glory::PixelFormat::PF_G16B16R163Plane422Unorm:
            break;
        case Glory::PixelFormat::PF_G16B16R162Plane422Unorm:
            break;
        case Glory::PixelFormat::PF_G16B16R163Plane444Unorm:
            break;
        case Glory::PixelFormat::PF_Pvrtc12BppUnormBlockIMG:
            break;
        case Glory::PixelFormat::PF_Pvrtc14BppUnormBlockIMG:
            break;
        case Glory::PixelFormat::PF_Pvrtc22BppUnormBlockIMG:
            break;
        case Glory::PixelFormat::PF_Pvrtc24BppUnormBlockIMG:
            break;
        case Glory::PixelFormat::PF_Pvrtc12BppSrgbBlockIMG:
            break;
        case Glory::PixelFormat::PF_Pvrtc14BppSrgbBlockIMG:
            break;
        case Glory::PixelFormat::PF_Pvrtc22BppSrgbBlockIMG:
            break;
        case Glory::PixelFormat::PF_Pvrtc24BppSrgbBlockIMG:
            break;
        case Glory::PixelFormat::PF_Astc4x4SfloatBlockEXT:
            break;
        case Glory::PixelFormat::PF_Astc5x4SfloatBlockEXT:
            break;
        case Glory::PixelFormat::PF_Astc5x5SfloatBlockEXT:
            break;
        case Glory::PixelFormat::PF_Astc6x5SfloatBlockEXT:
            break;
        case Glory::PixelFormat::PF_Astc6x6SfloatBlockEXT:
            break;
        case Glory::PixelFormat::PF_Astc8x5SfloatBlockEXT:
            break;
        case Glory::PixelFormat::PF_Astc8x6SfloatBlockEXT:
            break;
        case Glory::PixelFormat::PF_Astc8x8SfloatBlockEXT:
            break;
        case Glory::PixelFormat::PF_Astc10x5SfloatBlockEXT:
            break;
        case Glory::PixelFormat::PF_Astc10x6SfloatBlockEXT:
            break;
        case Glory::PixelFormat::PF_Astc10x8SfloatBlockEXT:
            break;
        case Glory::PixelFormat::PF_Astc10x10SfloatBlockEXT:
            break;
        case Glory::PixelFormat::PF_Astc12x10SfloatBlockEXT:
            break;
        case Glory::PixelFormat::PF_Astc12x12SfloatBlockEXT:
            break;
        case Glory::PixelFormat::PF_B10X6G10X6R10X6G10X6422Unorm4Pack16KHR:
            break;
        case Glory::PixelFormat::PF_B12X4G12X4R12X4G12X4422Unorm4Pack16KHR:
            break;
        case Glory::PixelFormat::PF_B16G16R16G16422UnormKHR:
            break;
        case Glory::PixelFormat::PF_B8G8R8G8422UnormKHR:
            break;
        case Glory::PixelFormat::PF_G10X6B10X6G10X6R10X6422Unorm4Pack16KHR:
            break;
        case Glory::PixelFormat::PF_G10X6B10X6R10X62Plane420Unorm3Pack16KHR:
            break;
        case Glory::PixelFormat::PF_G10X6B10X6R10X62Plane422Unorm3Pack16KHR:
            break;
        case Glory::PixelFormat::PF_G10X6B10X6R10X63Plane420Unorm3Pack16KHR:
            break;
        case Glory::PixelFormat::PF_G10X6B10X6R10X63Plane422Unorm3Pack16KHR:
            break;
        case Glory::PixelFormat::PF_G10X6B10X6R10X63Plane444Unorm3Pack16KHR:
            break;
        case Glory::PixelFormat::PF_G12X4B12X4G12X4R12X4422Unorm4Pack16KHR:
            break;
        case Glory::PixelFormat::PF_G12X4B12X4R12X42Plane420Unorm3Pack16KHR:
            break;
        case Glory::PixelFormat::PF_G12X4B12X4R12X42Plane422Unorm3Pack16KHR:
            break;
        case Glory::PixelFormat::PF_G12X4B12X4R12X43Plane420Unorm3Pack16KHR:
            break;
        case Glory::PixelFormat::PF_G12X4B12X4R12X43Plane422Unorm3Pack16KHR:
            break;
        case Glory::PixelFormat::PF_G12X4B12X4R12X43Plane444Unorm3Pack16KHR:
            break;
        case Glory::PixelFormat::PF_G16B16G16R16422UnormKHR:
            break;
        case Glory::PixelFormat::PF_G16B16R162Plane420UnormKHR:
            break;
        case Glory::PixelFormat::PF_G16B16R162Plane422UnormKHR:
            break;
        case Glory::PixelFormat::PF_G16B16R163Plane420UnormKHR:
            break;
        case Glory::PixelFormat::PF_G16B16R163Plane422UnormKHR:
            break;
        case Glory::PixelFormat::PF_G16B16R163Plane444UnormKHR:
            break;
        case Glory::PixelFormat::PF_G8B8G8R8422UnormKHR:
            break;
        case Glory::PixelFormat::PF_G8B8R82Plane420UnormKHR:
            break;
        case Glory::PixelFormat::PF_G8B8R82Plane422UnormKHR:
            break;
        case Glory::PixelFormat::PF_G8B8R83Plane420UnormKHR:
            break;
        case Glory::PixelFormat::PF_G8B8R83Plane422UnormKHR:
            break;
        case Glory::PixelFormat::PF_G8B8R83Plane444UnormKHR:
            break;
        case Glory::PixelFormat::PF_R10X6G10X6B10X6A10X6Unorm4Pack16KHR:
            break;
        case Glory::PixelFormat::PF_R10X6G10X6Unorm2Pack16KHR:
            break;
        case Glory::PixelFormat::PF_R10X6UnormPack16KHR:
            break;
        case Glory::PixelFormat::PF_R12X4G12X4B12X4A12X4Unorm4Pack16KHR:
            break;
        case Glory::PixelFormat::PF_R12X4G12X4Unorm2Pack16KHR:
            break;
        case Glory::PixelFormat::PF_R12X4UnormPack16KHR:
            break;
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