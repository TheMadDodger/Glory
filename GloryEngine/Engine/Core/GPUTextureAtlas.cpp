#include "GPUTextureAtlas.h"
#include "GraphicsDevice.h"

#include "Engine.h"
#include "Debug.h"

namespace Glory
{
	GPUTextureAtlas::GPUTextureAtlas(TextureCreateInfo&& createInfo, Engine* pEngine, TextureHandle texture):
		TextureAtlas(pEngine, createInfo.m_Width, createInfo.m_Height),
		m_TextureInfo(std::move(createInfo)), m_Texture(texture)
	{
	}

	GPUTextureAtlas::~GPUTextureAtlas()
	{
		m_Texture = NULL;
	}

	void GPUTextureAtlas::Initialize()
	{
		if (m_Texture) return;
		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
		m_Texture = pDevice->CreateTexture(m_TextureInfo);
	}

	TextureHandle GPUTextureAtlas::GetTexture() const
	{
		return m_Texture;
	}

	void GPUTextureAtlas::OnResize()
	{
		m_TextureInfo.m_Width = m_Width;
		m_TextureInfo.m_Height = m_Height;
		//m_pTexture->Resize(m_TextureInfo.m_Width, m_TextureInfo.m_Height);
	}
}
