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
		virtual void ReadColorPixel(const std::string& attachment, const glm::ivec2& coord, void* value, DataType type) override;
		virtual void ReadDepthPixel(const glm::ivec2& coord, void* value, DataType type) override;

	private:
		virtual void Initialize() override;
		virtual void OnResize() override;
		virtual void BindForDraw() override;
		virtual void BindRead() override;
		virtual void UnBindForDraw() override;
		virtual void UnBindRead() override;

	private:
		friend class OGLResourceManager;
		GLuint m_GLFrameBufferID;
	};
}
