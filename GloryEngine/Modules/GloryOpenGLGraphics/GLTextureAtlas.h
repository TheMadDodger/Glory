#pragma once
#include <TextureAtlas.h>

#include <memory>

namespace Glory
{
	class GLTexture;

	/** @todo Make API independant, there should be a GPUTextureAtlas and a CPUTextureAtlas instead */
	class GLTextureAtlas : public TextureAtlas
	{
	public:
		GLTextureAtlas(TextureCreateInfo&& createInfo, Engine* pEngine);
		virtual ~GLTextureAtlas();

	private:
		virtual void Initialize() override;
		virtual Texture* GetTexture() override;

		virtual bool AssignChunk(Texture* pTexture, const ReservedChunk& chunk) override;

	private:
		GLTexture* m_pTexture;
		unsigned int m_GLFrameBufferID;
	};
}
