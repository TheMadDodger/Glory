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
		Attachment(const std::string& name, const PixelFormat pixelFormat, const ImageType& imageType, const ImageAspect& imageAspect);

		//PixelFormat::PF_R8G8B8A8Srgb
		std::string Name;
		PixelFormat Format;
		ImageType ImageType;
		ImageAspect ImageAspect;
	};

	struct RenderTextureCreateInfo
	{
	public:
		RenderTextureCreateInfo(size_t width, size_t height, bool hasDepth);

		size_t Width;
		size_t Height;
		bool HasDepth;
		std::vector<Attachment> Attachments;
	};

	class RenderTexture : public GPUResource
	{
	public:
		void Resize(size_t width, size_t height);
		void GetDimensions(size_t& width, size_t& height) const;
		Texture* GetTextureAttachment(size_t index);
		Texture* GetTextureAttachment(const std::string& name);

		virtual void Bind() = 0;
		virtual void UnBind() = 0;

		void BindAll(Material* pMaterial);

	protected:
		RenderTexture(const RenderTextureCreateInfo& createInfo);
		virtual ~RenderTexture();

	protected:
		virtual void Initialize() = 0;
		virtual void OnResize() = 0;

		void CreateTextures(GPUResourceManager* pResourceManager);

	protected:
		friend class GPUResourceManager;
		size_t m_Width;
		size_t m_Height;
		std::vector<Texture*> m_pTextures;
		std::map<std::string, size_t> m_NameToTextureIndex;
		const RenderTextureCreateInfo m_CreateInfo;
	};
}
