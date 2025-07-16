#pragma once
#include "TextureAtlas.h"

#include <memory>

namespace Glory
{
	class RenderTexture;

	/** @brief Texture atlas for Open GL graphics API */
	class GPUTextureAtlas : public TextureAtlas
	{
	public:
		/** @brief Constructor
		 * @param createInfo Creation info for the attached texture
		 * @param pEngine Engine instance
		 */
		GPUTextureAtlas(TextureCreateInfo&& createInfo, Engine* pEngine, bool depth);
		/** @brief Destructor */
		virtual ~GPUTextureAtlas();

		/** @brief Get the attached texture */
		virtual Texture* GetTexture() override;
		/** @brief Initialize the atlas by creating the render texture */
		virtual void Initialize() override;

		/** @brief Bind texture for rendering */
		virtual void Bind() override;
		/** @brief Unbind texture */
		virtual void Unbind() override;

	private:
		/** @brief GPU implementation for copying pixels from a texture to a chunk in the atlas
		 * @param pTexture Texture to copy pixels from
		 * @param chunk The chunk data to copy the pixels to
		 * @returns @cpp true @ce on success, @cpp false @ce otherwise
		 */
		virtual bool AssignChunk(Texture* pTexture, const ReservedChunk& chunk) override;
		/** @brief GPU Implementation for binding a chunk for rendering
		 * @param chunk The chunk to bind for rendering
		 * @returns @cpp true @ce on success, @cpp false @ce otherwise
		 */
		virtual bool OnBindChunk(const ReservedChunk& chunk) override;
		/** @brief GPU implementation for resizing the atlas */
		virtual void OnResize() override;
		/** @brief GPU implementation for clearing the atlas */
		virtual void OnClear(const glm::vec4& clearColor, double depth) override;

	private:
		TextureCreateInfo m_TextureInfo;
		RenderTexture* m_pTexture;
		bool m_IsDepth;
	};
}
