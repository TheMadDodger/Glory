#include "GPUTextureAtlas.h"
#include "Texture.h"
#include "GraphicsModule.h"

#include "Engine.h"
#include "Debug.h"

namespace Glory
{
	GPUTextureAtlas::GPUTextureAtlas(TextureCreateInfo&& createInfo, Engine* pEngine, bool depth):
		TextureAtlas(pEngine, createInfo.m_Width, createInfo.m_Height),
		m_TextureInfo(std::move(createInfo)), m_pTexture(nullptr), m_IsDepth(depth)
	{
	}

	GPUTextureAtlas::~GPUTextureAtlas()
	{
		m_pTexture = nullptr;
	}

	void GPUTextureAtlas::Initialize()
	{
		GraphicsModule* pGraphics = m_pEngine->GetMainModule<GraphicsModule>();
		GPUResourceManager* pResources = pGraphics->GetResourceManager();

		RenderTextureCreateInfo renderTextureInfo;
		renderTextureInfo.HasDepth = m_IsDepth;
		renderTextureInfo.HasStencil = false;
		renderTextureInfo.Width = m_TextureInfo.m_Width;
		renderTextureInfo.Height = m_TextureInfo.m_Height;
		if (!m_IsDepth) renderTextureInfo.Attachments.emplace_back(Attachment{ "Color", m_TextureInfo });

		m_pTexture = pResources->CreateRenderTexture(renderTextureInfo);
	}

	void GPUTextureAtlas::Unbind()
	{
		GraphicsModule* pGraphics = m_pEngine->GetMainModule<GraphicsModule>();
		m_pTexture->UnBindForDraw();
	}

	Texture* GPUTextureAtlas::GetTexture()
	{
		return m_pTexture->GetTextureAttachment(0);
	}

	bool GPUTextureAtlas::AssignChunk(Texture* pTexture, const ReservedChunk& chunk)
	{
		GraphicsModule* pGraphics = m_pEngine->GetMainModule<GraphicsModule>();
		Material* pMaterial = pGraphics->UsePassthroughMaterial();

		m_pTexture->BindForDraw();
		pGraphics->SetViewport(int(chunk.XOffset), int(chunk.YOffset), chunk.Width, chunk.Height);

		pMaterial->SetTexture("Color", pTexture);
		pGraphics->DrawScreenQuad();

		m_pTexture->UnBindForDraw();
		pGraphics->UseMaterial(nullptr);
		return true;
	}

	bool GPUTextureAtlas::OnBindChunk(const ReservedChunk& chunk)
	{
		GraphicsModule* pGraphics = m_pEngine->GetMainModule<GraphicsModule>();
		m_pTexture->BindForDraw();
		pGraphics->SetViewport(int(chunk.XOffset), int(chunk.YOffset), chunk.Width, chunk.Height);
		pGraphics->Scissor(int(chunk.XOffset), int(chunk.YOffset), chunk.Width, chunk.Height);
		pGraphics->Clear();
		pGraphics->EndScissor();
		return true;
	}

	void GPUTextureAtlas::OnResize()
	{
		m_TextureInfo.m_Width = m_Width;
		m_TextureInfo.m_Height = m_Height;
		m_pTexture->Resize(m_TextureInfo.m_Width, m_TextureInfo.m_Height);
	}

	void GPUTextureAtlas::OnClear(const glm::vec4& clearColor, double depth)
	{
		GraphicsModule* pGraphics = m_pEngine->GetMainModule<GraphicsModule>();
		m_pTexture->BindForDraw();
		pGraphics->Clear(clearColor, depth);
		m_pTexture->UnBindForDraw();
	}
}
