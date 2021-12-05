#pragma once
#include <RenderTexture.h>
#include "GLTexture.h"

namespace Glory
{
	class OGLRenderTexture : public RenderTexture
	{
	private:
		OGLRenderTexture(int width, int height, bool hasDepthBuffer);
		virtual ~OGLRenderTexture();

	private:
		virtual void Initialize() override;
		virtual void OnResize() override;
		virtual void Bind() override;
		virtual void UnBind() override;

	private:
		friend class OGLResourceManager;
		GLuint m_GLFrameBufferID;
		GLuint m_GLDepthBufferID;
	};
}
