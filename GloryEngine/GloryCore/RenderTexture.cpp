#include "RenderTexture.h"
#include "Debug.h"

namespace Glory
{
	void RenderTexture::Resize(size_t width, size_t height)
	{
		m_Width = width;
		m_Height = height;
		OnResize();
	}

	void RenderTexture::GetDimensions(size_t& width, size_t& height)
	{
		width = m_Width;
		height = m_Height;
	}

	Texture* RenderTexture::GetTexture()
	{
		return m_pTexture;
	}

	RenderTexture::RenderTexture(size_t width, size_t height, bool hasDepthBuffer) : m_Width(width), m_Height(height), m_HasDepthBuffer(hasDepthBuffer), m_pTexture(nullptr)
	{
	}

	RenderTexture::~RenderTexture() {}
}