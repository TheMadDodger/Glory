#pragma once
#include "TextureAtlas.h"
#include "GraphicsHandles.h"
#include "GraphicsEnums.h"

#include <memory>

namespace Glory
{
	class RenderTexture;
	class GraphicsDevice;

	/** @brief Texture atlas for Open GL graphics API */
	class GPUTextureAtlas : public TextureAtlas
	{
	public:
		/** @brief Constructor
		 * @param createInfo Creation info for the attached texture
		 * @param pEngine Engine instance
		 */
		GPUTextureAtlas(TextureCreateInfo&& createInfo, Engine* pEngine, TextureHandle texture=0);
		/** @brief Destructor */
		virtual ~GPUTextureAtlas();

		/** @brief Get the attached texture */
		virtual TextureHandle GetTexture() override;
		/** @brief Initialize the atlas by creating the render texture */
		virtual void Initialize() override;

	private:
		/** @brief GPU implementation for resizing the atlas */
		virtual void OnResize() override;

	private:
		TextureCreateInfo m_TextureInfo;
		TextureHandle m_Texture;
	};
}
