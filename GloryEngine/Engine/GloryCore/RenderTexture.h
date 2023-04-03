#pragma once
#include <vector>
#include <list>
#include <glm/glm.hpp>
#include <map>
#include <string>
#include "Texture.h"
#include "GPUResource.h"
#include "Material.h"

namespace Glory
{
	class GPUResourceManager;

	struct Attachment
	{
		Attachment(const std::string& name, const PixelFormat& pixelFormat, const PixelFormat& internalFormat, const ImageType& imageType, const ImageAspect& imageAspect, bool autoBind = true);

		//PixelFormat::PF_R8G8B8A8Srgb
		std::string Name;
		PixelFormat InternalFormat;
		PixelFormat Format;
		ImageType ImageType;
		ImageAspect ImageAspect;
		bool m_AutoBind;
	};

	struct RenderTextureCreateInfo
	{
	public:
		RenderTextureCreateInfo(uint32_t width, uint32_t height, bool hasDepth);

		uint32_t Width;
		uint32_t Height;
		bool HasDepth;
		std::vector<Attachment> Attachments;
	};

	class RenderTexture : public GPUResource
	{
	public:
		void Resize(uint32_t width, uint32_t height);
		void GetDimensions(uint32_t& width, uint32_t& height) const;
		Texture* GetTextureAttachment(size_t index);
		Texture* GetTextureAttachment(const std::string& name);

		virtual void Bind() = 0;
		virtual void UnBind() = 0;

		void BindAll(Material* pMaterial);

		size_t AttachmentCount() const;
		const std::string AttachmentName(size_t index) const;

		virtual uint32_t ReadPixel(const glm::ivec2& coord) = 0;

	protected:
		RenderTexture(const RenderTextureCreateInfo& createInfo);
		virtual ~RenderTexture();

	protected:
		virtual void Initialize() = 0;
		virtual void OnResize() = 0;

		void CreateTextures(GPUResourceManager* pResourceManager);

	protected:
		friend class GPUResourceManager;
		uint32_t m_Width;
		uint32_t m_Height;
		std::vector<Texture*> m_pTextures;
		std::map<std::string, size_t> m_NameToTextureIndex;
		std::vector<std::string> m_Names;
		const RenderTextureCreateInfo m_CreateInfo;
	};
}
