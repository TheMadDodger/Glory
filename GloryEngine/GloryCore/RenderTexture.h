#pragma once
#include <vector>
#include <list>
#include <glm/glm.hpp>
#include "Texture.h"
#include "GPUResource.h"

namespace Glory
{
	class RenderTexture : public GPUResource
	{
	public:
		void Resize(size_t width, size_t height);
		void GetDimensions(size_t& width, size_t& height);
		Texture* GetTexture();
		//const GLuint& GetTextureID();

		virtual void Bind() = 0;
		virtual void UnBind() = 0;


	protected:
		RenderTexture(size_t width, size_t height, bool hasDepthBuffer);
		~RenderTexture();

	protected:
		virtual void Initialize() = 0;
		virtual void OnResize() = 0;

	protected:
		friend class GPUResourceManager;
		size_t m_Width;
		size_t m_Height;
		bool m_HasDepthBuffer;
		Texture* m_pTexture;
	};
}