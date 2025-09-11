#include "GPUTextureAtlas.h"
#include "Texture.h"
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

	void GPUTextureAtlas::Bind()
	{
	}

	void GPUTextureAtlas::Unbind()
	{
	}

	TextureHandle GPUTextureAtlas::GetTexture()
	{
		return m_Texture;
	}

	bool GPUTextureAtlas::AssignChunk(Texture* pTexture, const ReservedChunk& chunk)
	{
		//GraphicsModule* pGraphics = m_pEngine->GetMainModule<GraphicsModule>();
		//Material* pMaterial = pGraphics->UsePassthroughMaterial();
		//
		//m_pTexture->BindForDraw();
		//pGraphics->SetViewport(int(chunk.XOffset), int(chunk.YOffset), chunk.Width, chunk.Height);
		//
		//pMaterial->SetTexture("Color", pTexture);
		//pGraphics->DrawScreenQuad();
		//
		//m_pTexture->UnBindForDraw();
		//pGraphics->UseMaterial(nullptr);
		return true;
	}

	bool GPUTextureAtlas::OnBindChunk(const ReservedChunk& chunk)
	{
		//GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
		//GraphicsModule* pGraphics = m_pEngine->GetMainModule<GraphicsModule>();
		//pGraphics->SetViewport(int(chunk.XOffset), int(chunk.YOffset), chunk.Width, chunk.Height);
		//pGraphics->Scissor(int(chunk.XOffset), int(chunk.YOffset), chunk.Width, chunk.Height);
		//pGraphics->Clear();
		//pGraphics->EndScissor();
		return true;
	}

	void GPUTextureAtlas::OnResize()
	{
		//m_TextureInfo.m_Width = m_Width;
		//m_TextureInfo.m_Height = m_Height;
		//m_pTexture->Resize(m_TextureInfo.m_Width, m_TextureInfo.m_Height);
	}

	void GPUTextureAtlas::OnClear(const glm::vec4& clearColor, double depth)
	{
		//GraphicsModule* pGraphics = m_pEngine->GetMainModule<GraphicsModule>();
		//m_pTexture->BindForDraw();
		//pGraphics->Clear(clearColor, depth);
		//m_pTexture->UnBindForDraw();
	}
}
