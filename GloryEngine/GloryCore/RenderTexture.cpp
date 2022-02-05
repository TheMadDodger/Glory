#include "RenderTexture.h"
#include "Debug.h"
#include "GPUResourceManager.h"
#include "GraphicsEnums.h"

namespace Glory
{
	Attachment::Attachment(const std::string& name, const PixelFormat pixelFormat, const Glory::ImageType& imageType, const Glory::ImageAspect& imageAspect)
		: Name(name), Format(pixelFormat), ImageType(imageType), ImageAspect(imageAspect)
	{
	}

	RenderTextureCreateInfo::RenderTextureCreateInfo(size_t width, size_t height, bool hasDepth)
		: Width(width), Height(height), HasDepth(hasDepth), Attachments(std::vector<Attachment>()) {}

	void RenderTexture::Resize(size_t width, size_t height)
	{
		m_Width = width;
		m_Height = height;
		OnResize();
	}

	void RenderTexture::GetDimensions(size_t& width, size_t& height) const
	{
		width = m_Width;
		height = m_Height;
	}

	Texture* RenderTexture::GetTextureAttachment(size_t index)
	{
		return m_pTextures[index];
	}
	
	Texture* RenderTexture::GetTextureAttachment(const std::string& name)
	{
		if (m_NameToTextureIndex.find(name) == m_NameToTextureIndex.end()) return nullptr;
		size_t index = m_NameToTextureIndex[name];
		return m_pTextures[index];
	}

	void RenderTexture::BindAll(Material* pMaterial)
	{
		auto it = m_NameToTextureIndex.begin();
		for (size_t i = 0; i < m_CreateInfo.Attachments.size(); i++)
		{
			std::string name = (*it).first;
			pMaterial->SetTexture(name, m_pTextures[i]);
			++it;
		}

		if (m_CreateInfo.HasDepth) pMaterial->SetTexture("Depth", GetTextureAttachment("Depth"));
	}

	RenderTexture::RenderTexture(const RenderTextureCreateInfo& createInfo)
		: m_CreateInfo(createInfo), m_Width(createInfo.Width), m_Height(createInfo.Height),
		m_pTextures(std::vector<Texture*>(createInfo.Attachments.size() + (createInfo.HasDepth ? 1 : 0))),
		m_NameToTextureIndex(std::map<std::string, size_t>())
	{
	}

	RenderTexture::~RenderTexture()
	{
		m_pTextures.clear();
		m_NameToTextureIndex.clear();
	}

	void RenderTexture::CreateTextures(GPUResourceManager* pResourceManager)
	{
		for (size_t i = 0; i < m_CreateInfo.Attachments.size(); i++)
		{
			Attachment attachment = m_CreateInfo.Attachments[i];
			Texture* pTexture = pResourceManager->CreateTexture(m_Width, m_Height, attachment.Format, attachment.ImageType, 0, 0, attachment.ImageAspect, SamplerSettings());
			m_pTextures[i] = pTexture;
			m_NameToTextureIndex[attachment.Name] = i;
		}

		if (!m_CreateInfo.HasDepth) return;
		size_t depthIndex = m_CreateInfo.Attachments.size();

		//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, m_Width, m_Height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);

		Texture* pDepthTexture = pResourceManager->CreateTexture(m_Width, m_Height, PixelFormat::PF_Depth24, ImageType::IT_2D, 0, 0, ImageAspect::IA_Depth, SamplerSettings());
		m_pTextures[depthIndex] = pDepthTexture;
		m_NameToTextureIndex["Depth"] = depthIndex;
	}
}