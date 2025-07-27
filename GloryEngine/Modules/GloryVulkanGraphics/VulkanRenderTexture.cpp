#include "VulkanRenderTexture.h"
#include "VulkanGraphicsModule.h"
#include "Device.h"

#include <Engine.h>

namespace Glory
{
	VulkanRenderTexture::VulkanRenderTexture(const RenderTextureCreateInfo& createInfo):
		RenderTexture(createInfo), m_Framebuffer(nullptr)
	{
	}

	VulkanRenderTexture::~VulkanRenderTexture()
	{
		VulkanGraphicsModule* pGraphics = m_pOwner->GetEngine()->GetMainModule<VulkanGraphicsModule>();
		VulkanDeviceManager& deviceManager = pGraphics->GetDeviceManager();
		Device* pDevice = deviceManager.GetSelectedDevice();
		LogicalDeviceData deviceData = pDevice->GetLogicalDeviceData();
		deviceData.LogicalDevice.destroyFramebuffer(m_Framebuffer);
	}

	void VulkanRenderTexture::ReadColorPixel(const std::string& attachment, const glm::ivec2& coord, void* value, DataType type)
	{
		throw new std::exception("VulkanRenderTexture::ReadColorPixel() not yet implemented!");
	}

	void VulkanRenderTexture::ReadColorPixels(const std::string& attachment, void* buffer, DataType type)
	{
		throw new std::exception("VulkanRenderTexture::ReadColorPixels() not yet implemented!");
	}

	void VulkanRenderTexture::ReadDepthPixel(const glm::ivec2& coord, void* value, DataType type)
	{
		throw new std::exception("VulkanRenderTexture::ReadDepthPixel() not yet implemented!");
	}

	void VulkanRenderTexture::Initialize()
	{
		CreateTextures();

		VulkanGraphicsModule* pGraphics = m_pOwner->GetEngine()->GetMainModule<VulkanGraphicsModule>();
		VulkanDeviceManager& deviceManager = pGraphics->GetDeviceManager();
		Device* pDevice = deviceManager.GetSelectedDevice();
		LogicalDeviceData deviceData = pDevice->GetLogicalDeviceData();

		std::vector<vk::ImageView> attachments(m_pTextures.size());
		for (size_t i = 0; i < m_pTextures.size(); ++i)
		{
			VulkanTexture* pVulkanTexture = static_cast<VulkanTexture*>(m_pTextures[i]);
			attachments[i] = pVulkanTexture->GetTextureImageView();
		}
		                
		vk::FramebufferCreateInfo frameBufferCreateInfo = vk::FramebufferCreateInfo()
		    .setRenderPass(pGraphics->MainRenderPass().GetRenderPass())
		    .setAttachmentCount(attachments.size())
		    .setPAttachments(attachments.data())
		    .setWidth(m_CreateInfo.Width)
		    .setHeight(m_CreateInfo.Height)
		    .setLayers(1);
		                
		m_Framebuffer = deviceData.LogicalDevice.createFramebuffer(frameBufferCreateInfo);
		if (m_Framebuffer == nullptr)
		    throw std::runtime_error("failed to create framebuffer!");
	}

	void VulkanRenderTexture::OnResize()
	{
		VulkanGraphicsModule* pGraphics = m_pOwner->GetEngine()->GetMainModule<VulkanGraphicsModule>();
		VulkanDeviceManager& deviceManager = pGraphics->GetDeviceManager();
		Device* pDevice = deviceManager.GetSelectedDevice();
		LogicalDeviceData deviceData = pDevice->GetLogicalDeviceData();

		if (m_Framebuffer != nullptr) deviceData.LogicalDevice.destroyFramebuffer(m_Framebuffer);
		m_Framebuffer = nullptr;

		GPUResourceManager* pResourceManager = m_pOwner->GetEngine()->GetMainModule<GraphicsModule>()->GetResourceManager();
		size_t attachmentCount = m_CreateInfo.Attachments.size();
		for (size_t i = 0; i < attachmentCount; i++)
		{
			pResourceManager->Free(m_pTextures[i]);
		}
		if (m_CreateInfo.HasDepth)
		{
			pResourceManager->Free(m_pTextures[attachmentCount]);
			++attachmentCount;
		}
		if (m_CreateInfo.HasStencil) pResourceManager->Free(m_pTextures[attachmentCount]);
		Initialize();
	}

	void VulkanRenderTexture::BindForDraw()
	{
		throw new std::exception("VulkanRenderTexture::BindForDraw() not yet implemented!");
	}

	void VulkanRenderTexture::BindRead()
	{
		throw new std::exception("VulkanRenderTexture::BindRead() not yet implemented!");
	}

	void VulkanRenderTexture::UnBindForDraw()
	{
		throw new std::exception("VulkanRenderTexture::UnBindForDraw() not yet implemented!");
	}

	void VulkanRenderTexture::UnBindRead()
	{
		throw new std::exception("VulkanRenderTexture::UnBindRead() not yet implemented!");
	}
}
