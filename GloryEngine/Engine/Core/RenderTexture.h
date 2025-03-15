#pragma once
#include "GPUResource.h"
#include "GraphicsEnums.h"

#include <vector>
#include <list>
#include <glm/glm.hpp>
#include <map>
#include <string>

namespace Glory
{
	class GPUResourceManager;
	class Texture;
	class Material;

	struct Attachment
	{
		Attachment(const std::string& name, const PixelFormat& pixelFormat, const PixelFormat& internalFormat,
			const ImageType& imageType, const ImageAspect& imageAspect, DataType type=DataType::DT_UByte, bool autoBind = true);

		//PixelFormat::PF_R8G8B8A8Srgb
		std::string Name;
		PixelFormat InternalFormat;
		PixelFormat Format;
		ImageType ImageType;
		ImageAspect ImageAspect;
		DataType m_Type;
		bool m_AutoBind;
	};

	struct RenderTextureCreateInfo
	{
	public:
		RenderTextureCreateInfo();
		RenderTextureCreateInfo(uint32_t width, uint32_t height, bool hasDepth, bool hasStencil = false);

		uint32_t Width;
		uint32_t Height;
		bool HasDepth;
		bool HasStencil;
		std::vector<Attachment> Attachments;
	};

	class RenderTexture : public GPUResource
	{
	public:
		void Resize(uint32_t width, uint32_t height);
		void GetDimensions(uint32_t& width, uint32_t& height) const;
		Texture* GetTextureAttachment(size_t index);
		Texture* GetTextureAttachment(const std::string& name);

		virtual void BindForDraw() = 0;
		virtual void BindRead() = 0;
		virtual void UnBindForDraw() = 0;
		virtual void UnBindRead() = 0;

		void BindAll(Material* pMaterial);
		//void Bind(size_t index);

		size_t AttachmentCount() const;
		const std::string AttachmentName(size_t index) const;

		virtual void ReadColorPixel(const std::string& attachment, const glm::ivec2& coord, void* value, DataType type) = 0;
		virtual void ReadDepthPixel(const glm::ivec2& coord, void* value, DataType type) = 0;

	protected:
		RenderTexture(const RenderTextureCreateInfo& createInfo);
		virtual ~RenderTexture();

	protected:
		virtual void Initialize() = 0;
		virtual void OnResize() = 0;

		void CreateTextures();

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
