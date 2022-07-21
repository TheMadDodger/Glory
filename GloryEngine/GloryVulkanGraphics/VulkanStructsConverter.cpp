#include "VulkanStructsConverter.h"

namespace Glory
{
    const std::map<BufferBindingTarget, vk::BufferUsageFlags> VKConverter::TO_BUFFERUSAGE
    {

    };

    vk::ImageViewType VKConverter::GetVulkanImageViewType(const ImageType& imageType)
    {
        switch (imageType)
        {
        case Glory::ImageType::IT_1D:
            return vk::ImageViewType::e1D;
        case Glory::ImageType::IT_2D:
            return vk::ImageViewType::e2D;
        case Glory::ImageType::IT_3D:
            return vk::ImageViewType::e3D;
        case Glory::ImageType::IT_Cube:
            return vk::ImageViewType::eCube;
        case Glory::ImageType::IT_1DArray:
            return vk::ImageViewType::e1DArray;
        case Glory::ImageType::IT_2DArray:
            return vk::ImageViewType::e2DArray;
        case Glory::ImageType::IT_CubeArray:
            return vk::ImageViewType::eCubeArray;
        }

        return (vk::ImageViewType)0;
    }

    vk::ImageType VKConverter::GetVulkanImageType(const ImageType& imageType)
    {
        switch (imageType)
        {
        case Glory::ImageType::IT_1D:
            return vk::ImageType::e1D;
        case Glory::ImageType::IT_2D:
            return vk::ImageType::e2D;
        case Glory::ImageType::IT_3D:
            return vk::ImageType::e3D;
        }

        return (vk::ImageType)0;
    }

    vk::ImageAspectFlags VKConverter::GetVulkanImageAspectFlags(const ImageAspect& aspectFlags)
    {
        vk::ImageAspectFlags flags = vk::ImageAspectFlags();

        if ((aspectFlags & ImageAspect::IA_Color) > 0) flags |= vk::ImageAspectFlagBits::eColor;
        if ((aspectFlags & ImageAspect::IA_Depth) > 0) flags |= vk::ImageAspectFlagBits::eDepth;
        if ((aspectFlags & ImageAspect::IA_Metadata) > 0) flags |= vk::ImageAspectFlagBits::eMetadata;
        if ((aspectFlags & ImageAspect::IA_Plane0) > 0) flags |= vk::ImageAspectFlagBits::ePlane0;
        if ((aspectFlags & ImageAspect::IA_Plane1) > 0) flags |= vk::ImageAspectFlagBits::ePlane1;
        if ((aspectFlags & ImageAspect::IA_Plane2) > 0) flags |= vk::ImageAspectFlagBits::ePlane2;
        if ((aspectFlags & ImageAspect::IA_Plane0KHR) > 0) flags |= vk::ImageAspectFlagBits::ePlane0KHR;
        if ((aspectFlags & ImageAspect::IA_Plane1KHR) > 0) flags |= vk::ImageAspectFlagBits::ePlane1KHR;
        if ((aspectFlags & ImageAspect::IA_Plane2KHR) > 0) flags |= vk::ImageAspectFlagBits::ePlane2KHR;
        if ((aspectFlags & ImageAspect::IA_MemoryPlane0EXT) > 0) flags |= vk::ImageAspectFlagBits::eMemoryPlane0EXT;
        if ((aspectFlags & ImageAspect::IA_MemoryPlane1EXT) > 0) flags |= vk::ImageAspectFlagBits::eMemoryPlane1EXT;
        if ((aspectFlags & ImageAspect::IA_MemoryPlane2EXT) > 0) flags |= vk::ImageAspectFlagBits::eMemoryPlane2EXT;
        if ((aspectFlags & ImageAspect::IA_MemoryPlane3EXT) > 0) flags |= vk::ImageAspectFlagBits::eMemoryPlane3EXT;
        if ((aspectFlags & ImageAspect::IA_Stencil) > 0) flags |= vk::ImageAspectFlagBits::eStencil;

        return flags;
    }

    vk::Filter VKConverter::GetVulkanFilter(const Filter& filter)
    {
        switch (filter)
        {
        case Glory::Filter::F_Nearest:
            return vk::Filter::eNearest;
        case Glory::Filter::F_Linear:
            return vk::Filter::eLinear;
        case Glory::Filter::F_CubicIMG:
            return vk::Filter::eCubicIMG;
        case Glory::Filter::F_CubicEXT:
            return vk::Filter::eCubicEXT;
        }

        return (vk::Filter)0;
    }

    vk::CompareOp VKConverter::GetVulkanCompareOp(const CompareOp& op)
    {
        switch (op)
        {
        case Glory::CompareOp::OP_Never:
            return vk::CompareOp::eNever;
        case Glory::CompareOp::OP_Less:
            return vk::CompareOp::eLess;
        case Glory::CompareOp::OP_Equal:
            return vk::CompareOp::eEqual;
        case Glory::CompareOp::OP_LessOrEqual:
            return vk::CompareOp::eLessOrEqual;
        case Glory::CompareOp::OP_Greater:
            return vk::CompareOp::eGreater;
        case Glory::CompareOp::OP_NotEqual:
            return vk::CompareOp::eNotEqual;
        case Glory::CompareOp::OP_GreaterOrEqual:
            return vk::CompareOp::eGreaterOrEqual;
        case Glory::CompareOp::OP_Always:
            return vk::CompareOp::eAlways;
        }
        return (vk::CompareOp)0;
    }

    vk::SamplerMipmapMode VKConverter::GetVulkanSamplerMipmapMode(const Filter& filter)
    {
        switch (filter)
        {
        case Glory::Filter::F_Nearest:
            return vk::SamplerMipmapMode::eNearest;
        case Glory::Filter::F_Linear:
            vk::SamplerMipmapMode::eLinear;
        }

        return (vk::SamplerMipmapMode)0;
    }

    vk::SamplerAddressMode VKConverter::GetSamplerAddressMode(const SamplerAddressMode& mode)
    {
        switch (mode)
        {
        case SamplerAddressMode::SAM_Repeat:
            return vk::SamplerAddressMode::eRepeat;
        case SamplerAddressMode::SAM_MirroredRepeat:
            return vk::SamplerAddressMode::eMirroredRepeat;
        case SamplerAddressMode::SAM_ClampToEdge:
            return vk::SamplerAddressMode::eClampToEdge;
        case SamplerAddressMode::SAM_ClampToBorder:
            return vk::SamplerAddressMode::eClampToBorder;
        case SamplerAddressMode::SAM_MirrorClampToEdge:
            return vk::SamplerAddressMode::eMirrorClampToEdge;
        case SamplerAddressMode::SAM_MirrorClampToEdgeKHR:
            return vk::SamplerAddressMode::eMirrorClampToEdgeKHR;
        }

        return (vk::SamplerAddressMode)0;
    }

    vk::SamplerCreateInfo VKConverter::GetVulkanSamplerInfo(const SamplerSettings& settings)
    {
        vk::SamplerCreateInfo samplerInfo = vk::SamplerCreateInfo();
        samplerInfo.magFilter = GetVulkanFilter(settings.MagFilter);
        samplerInfo.minFilter = GetVulkanFilter(settings.MinFilter);
        samplerInfo.addressModeU = GetSamplerAddressMode(settings.AddressModeU);
        samplerInfo.addressModeV = GetSamplerAddressMode(settings.AddressModeV);
        samplerInfo.addressModeW = GetSamplerAddressMode(settings.AddressModeW);
        samplerInfo.anisotropyEnable = settings.AnisotropyEnable ? VK_TRUE : VK_FALSE;
        samplerInfo.maxAnisotropy = settings.MaxAnisotropy;
        samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
        samplerInfo.unnormalizedCoordinates = settings.UnnormalizedCoordinates ? VK_TRUE : VK_FALSE;
        samplerInfo.compareEnable = settings.CompareEnable ? VK_TRUE : VK_FALSE;
        samplerInfo.compareOp = GetVulkanCompareOp(settings.CompareOp);
        samplerInfo.mipmapMode = GetVulkanSamplerMipmapMode(settings.MipmapMode);
        samplerInfo.mipLodBias = settings.MipLODBias;
        samplerInfo.minLod = settings.MinLOD;
        samplerInfo.maxLod = settings.MaxLOD;
        return samplerInfo;
    }

    vk::Format VKConverter::GetVulkanFormat(const PixelFormat& format)
    {
        int f = (int)format;
        if (f >= 1000156000)
            throw std::runtime_error("Format not supported");
        return (vk::Format)format;
    }

    vk::ShaderStageFlagBits VKConverter::GetShaderStageFlag(const ShaderType& shaderType)
    {
        switch (shaderType)
        {
        case ShaderType::ST_Vertex:
            return vk::ShaderStageFlagBits::eVertex;
        case ShaderType::ST_Fragment:
            return vk::ShaderStageFlagBits::eFragment;
        case ShaderType::ST_Geomtery:
            return vk::ShaderStageFlagBits::eGeometry;
        case ShaderType::ST_TessControl:
            return vk::ShaderStageFlagBits::eTessellationControl;
        case ShaderType::ST_TessEval:
            return vk::ShaderStageFlagBits::eTessellationEvaluation;
        case ShaderType::ST_Compute:
            return vk::ShaderStageFlagBits::eCompute;
        }

        return vk::ShaderStageFlagBits();
    }
}
