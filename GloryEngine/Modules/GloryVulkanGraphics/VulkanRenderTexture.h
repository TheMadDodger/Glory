#pragma once
#include <RenderTexture.h>

#include <vulkan/vulkan.hpp>

namespace Glory
{
	class VulkanRenderTexture : public RenderTexture
	{
	private:
		VulkanRenderTexture(const RenderTextureCreateInfo& createInfo);
		virtual ~VulkanRenderTexture();
		virtual void ReadColorPixel(const std::string& attachment, const glm::ivec2& coord, void* value, DataType type) override;
		virtual void ReadColorPixels(const std::string& attachment, void* buffer, DataType type) override;
		virtual void ReadDepthPixel(const glm::ivec2& coord, void* value, DataType type) override;

	private:
		virtual void Initialize() override;
		virtual void OnResize() override;
		virtual void BindForDraw() override;
		virtual void BindRead() override;
		virtual void UnBindForDraw() override;
		virtual void UnBindRead() override;

	private:
		friend class VulkanResourceManager;

		vk::Framebuffer m_Framebuffer;
	};
}
