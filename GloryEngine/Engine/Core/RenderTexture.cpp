#include "RenderTexture.h"
#include "Debug.h"
#include "GPUResourceManager.h"
#include "GraphicsEnums.h"
#include "Texture.h"
#include "Material.h"
#include <GL/glew.h>

namespace Glory
{
	Attachment::Attachment(const std::string& name, const PixelFormat& pixelFormat, const PixelFormat& internalFormat,
		const Glory::ImageType& imageType, const Glory::ImageAspect& imageAspect, DataType type, bool autoBind)
		: Name(name), InternalFormat(internalFormat), Format(pixelFormat), ImageType(imageType),
		ImageAspect(imageAspect), m_Type(type), m_AutoBind(autoBind)
	{
	}

	RenderTextureCreateInfo::RenderTextureCreateInfo()
		: Width(0), Height(0), HasDepth(false), HasStencil(false), Attachments(std::vector<Attachment>()) {}

	RenderTextureCreateInfo::RenderTextureCreateInfo(uint32_t width, uint32_t height, bool hasDepth, bool hasStencil)
		: Width(width), Height(height), HasDepth(hasDepth), HasStencil(hasStencil), Attachments(std::vector<Attachment>()) {}

	void RenderTexture::Resize(uint32_t width, uint32_t height)
	{
		m_Width = width;
		m_Height = height;
		OnResize();
	}

	void RenderTexture::GetDimensions(uint32_t& width, uint32_t& height) const
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
		for (size_t i = 0; i < m_CreateInfo.Attachments.size(); i++)
		{
			if (!m_CreateInfo.Attachments[i].m_AutoBind) continue;
			const std::string& name = m_CreateInfo.Attachments[i].Name;
			pMaterial->SetTexture(name, m_pTextures[i]);
		}
		if (m_CreateInfo.HasDepth) pMaterial->SetTexture("Depth", GetTextureAttachment("Depth"));
		if (m_CreateInfo.HasStencil) pMaterial->SetTexture("Stencil", GetTextureAttachment("Stencil"));
	}

	size_t RenderTexture::AttachmentCount() const
	{
		return m_pTextures.size();
	}

	const std::string RenderTexture::AttachmentName(size_t index) const
	{
		return m_Names[index];
	}

	RenderTexture::RenderTexture(const RenderTextureCreateInfo& createInfo)
		: m_CreateInfo(createInfo), m_Width(createInfo.Width), m_Height(createInfo.Height),
		m_pTextures(std::vector<Texture*>(createInfo.Attachments.size() + (createInfo.HasDepth ? 1 : 0) + (createInfo.HasStencil ? 1 : 0))),
		m_NameToTextureIndex(std::map<std::string, size_t>())
	{
	}

	RenderTexture::~RenderTexture()
	{
		m_pTextures.clear();
		m_NameToTextureIndex.clear();
	}

	void RenderTexture::CreateTextures()
	{
		m_Names.resize(m_CreateInfo.Attachments.size() + (m_CreateInfo.HasDepth ? 1 : 0) + (m_CreateInfo.HasStencil ? 1 : 0));

		SamplerSettings sampler;
		sampler.MipmapMode = Filter::F_None;
		sampler.MinFilter = Filter::F_Nearest;
		sampler.MagFilter = Filter::F_Nearest;

		size_t textureCounter = 0;
		for (size_t i = 0; i < m_CreateInfo.Attachments.size(); ++i)
		{
			Attachment attachment = m_CreateInfo.Attachments[i];
			Texture* pTexture = m_pOwner->CreateTexture({ m_Width, m_Height, attachment.Format, attachment.InternalFormat, attachment.ImageType, attachment.m_Type, 0, 0, attachment.ImageAspect, sampler });
			m_pTextures[i] = pTexture;
			m_NameToTextureIndex[attachment.Name] = i;
			m_Names[i] = attachment.Name;
			++textureCounter;
		}

		if (m_CreateInfo.HasDepth)
		{
			const size_t depthIndex = textureCounter;
			Texture* pDepthTexture = m_pOwner->CreateTexture({ m_Width, m_Height, PixelFormat::PF_Depth, PixelFormat::PF_Depth32, ImageType::IT_2D, DataType::DT_UInt, 0, 0, ImageAspect::IA_Depth, sampler });
			m_pTextures[depthIndex] = pDepthTexture;
			m_NameToTextureIndex["Depth"] = depthIndex;
			m_Names[depthIndex] = "Depth";
			++textureCounter;
		}

		if (m_CreateInfo.HasStencil)
		{
			const size_t stencilIndex = textureCounter;
			Texture* pDepthTexture = m_pOwner->CreateTexture({ m_Width, m_Height, PixelFormat::PF_Stencil, PixelFormat::PF_R8Uint, ImageType::IT_2D, DataType::DT_UInt, 0, 0, ImageAspect::IA_Stencil, sampler });
			m_pTextures[stencilIndex] = pDepthTexture;
			m_NameToTextureIndex["Stencil"] = stencilIndex;
			m_Names[stencilIndex] = "Stencil";
			++textureCounter;
		}
	}
}