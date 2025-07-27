#pragma once
#include <vulkan/vulkan.hpp>
#include <GraphicsEnums.h>
#include <map>

namespace Glory
{
	class VKConverter
	{
	public:
		static vk::ImageViewType GetVulkanImageViewType(const ImageType& imageType);
		static vk::ImageType GetVulkanImageType(const ImageType& imageType);
		static vk::ImageUsageFlags GetVulkanImageUsageFlags(const ImageAspect& aspectFlags);
		static vk::ImageAspectFlags GetVulkanImageAspectFlags(const ImageAspect& aspectFlags);
		static vk::Filter GetVulkanFilter(const Filter& filter);
		static vk::CompareOp GetVulkanCompareOp(const CompareOp& op);
		static vk::SamplerMipmapMode GetVulkanSamplerMipmapMode(const Filter& filter);
		static vk::SamplerAddressMode GetSamplerAddressMode(const SamplerAddressMode& mode);
		static vk::SamplerCreateInfo GetVulkanSamplerInfo(const SamplerSettings& settings);
		static vk::Format GetVulkanFormat(const PixelFormat& format);
		static vk::ShaderStageFlagBits GetShaderStageFlag(const ShaderType& shaderType);
		static vk::BufferUsageFlags ToBufferUsageFlags(const BufferBindingTarget& bindingTarget);

		//static const std::map<MemoryUsage, GLuint> TO_GLBUFFERUSAGE;

	private:
		VKConverter() {}
		virtual ~VKConverter() {}
	};
}