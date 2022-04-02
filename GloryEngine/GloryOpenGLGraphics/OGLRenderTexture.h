#pragma once
#include <RenderTexture.h>
#include "GLTexture.h"

namespace Glory
{
	class OGLRenderTexture : public RenderTexture
	{
	private:
		OGLRenderTexture(const RenderTextureCreateInfo& createInfo);
		virtual ~OGLRenderTexture();
		virtual uint32_t ReadPixel(const glm::ivec2& coord) override;

	private:
		virtual void Initialize() override;
		virtual void OnResize() override;
		virtual void Bind() override;
		virtual void UnBind() override;

	private:
		friend class OGLResourceManager;
		GLuint m_GLFrameBufferID;
	};
}
