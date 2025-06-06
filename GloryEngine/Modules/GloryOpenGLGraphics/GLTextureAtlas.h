#pragma once
#include <TextureAtlas.h>

#include <memory>

namespace Glory
{
	class GLTexture;

	/** @brief Texture atlas for Open GL graphics API */
	class GLTextureAtlas : public TextureAtlas
	{
	public:
		/** @brief Constructor
		 * @param createInfo Creation info for the attached texture
		 * @param pEngine Engine instance
		 */
		GLTextureAtlas(TextureCreateInfo&& createInfo, Engine* pEngine);
		/** @brief Destructor */
		virtual ~GLTextureAtlas();

	private:
		/** @brief Initialize the atlas by creating the render texture */
		virtual void Initialize() override;
		/** @brief Get the attached texture */
		virtual Texture* GetTexture() override;

		/** @brief OpenGL implementation for copying pixels from a texture to a chunk in the atlas
		 * @param pTexture Texture to copy pixels from
		 * @param chunk The chunk data to copy the pixels to
		 * @returns @cpp true @ce on success, @cpp false @ce otherwise
		 */
		virtual bool AssignChunk(Texture* pTexture, const ReservedChunk& chunk) override;

	private:
		GLTexture* m_pTexture;
		unsigned int m_GLFrameBufferID;
	};
}
