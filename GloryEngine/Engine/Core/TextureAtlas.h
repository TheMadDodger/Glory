#pragma once
#include "Texture.h"

namespace Glory
{
	class Texture;
	class Engine;

	/** @brief Texture atlas */
	class TextureAtlas
	{
	public:
		/** @brief Constructor
		 * @param pEngine Engine instance
		 * @param width Width of the atlas
		 * @param width Height of the atlas
		 */
		TextureAtlas(Engine* pEngine, uint32_t width, uint32_t height);
		/** @brief Destructor */
		virtual ~TextureAtlas();

		/** @brief Initialize the atlas by creating the nescesary resources */
		virtual void Initialize() = 0;
		/** @brief Get the GPU texture resource if available, otherwise nullptr */
		virtual Texture* GetTexture() = 0;

		/** @brief Reserver a chunk in the atlas
		 * @param width Width of the chunk
		 * @param height Height of the chunk
		 * @param id ID of the chunk
		 * @returns The ID of the chunk on success, 0 on fail
		 *
		 * Chunks are reserved in rows first, where each row has a consistent height.
		 * If a new row is required for the chosen height, reservation will fail if the
		 * remaining available height is smaller than the requested height.
		 */
		UUID ReserveChunk(uint32_t width, uint32_t height, UUID id=UUID());
		/** @brief Check if the atlas has a reserved chunk with an id
		 * @param id ID of the chunk to find
		 */
		bool HasReservedChunk(UUID id) const;
		/** @brief Copy the pixels from a GPU texture to a chunk in this atlas
		 * @param id ID of the chunk to copy to
		 * @param id pTexture The GPU texture to copy
		 * @returns @cpp true @ce on success, @cpp false @ce otherwise
		 *
		 * The texture must have the exact same size as the chunk, and its format
		 * must be compatible with the format of that atlas.
		 */
		bool AsignChunk(UUID id, Texture* pTexture);
		/** @brief Get the texture coordinates of a chunk
		 * @param id ID of the chunk
		 */
		glm::vec4 GetChunkCoords(UUID id) const;

		/** @brief Release a specific chunk from the atlas so that it can be reserved again
		 * @param id ID of the chunk to release
		 *
		 * Does not clear the attached texture
		 */
		void ReleaseChunk(UUID id);
		/** @brief Release all chunks and reset rows in the texture atlas
		 *
		 * Does not clear the attached texture
		 */
		void ReleaseAllChunks();

		/** @brief Resize the texture atlas
		 * @param newSize New width and height of the atlas
		 *
		 * Note: this releases all chunks!
		 */
		void Resize(uint32_t newSize);

		/** @brief Reset all pixels of the atlas to a specific color */
		void Clear(const glm::vec4& clearColor=glm::vec4{0.0f, 0.0f, 0.0f, 1.0f});

	protected:
		/** @brief Reserved chunk data */
		struct ReservedChunk
		{
			UUID ID;
			uint32_t XOffset;
			uint32_t YOffset;
			uint32_t Width;
			uint32_t Height;
			uint32_t RowIndex;
		};

		/** @brief Row data */
		struct RowData
		{
			uint32_t YOffset;
			uint32_t AvailableWidth;
			uint32_t Height;

			std::vector<std::pair<uint32_t, uint32_t>> m_FreeGaps;
		};

		/** @brief Implementation for copying the pixels of a GPU texture to a chunk in this atlas
		 * @param pTexture Texture to copy pixels from
		 * @param chunk The chunk data to copy the pixels to
		 * @returns @cpp true @ce on success, @cpp false @ce otherwise
		 */
		virtual bool AssignChunk(Texture* pTexture, const ReservedChunk& chunk) = 0;
		/** @brief Implementation for resizing the atlas */
		virtual void OnResize() = 0;
		/** @brief Implementation for clearing the atlas */
		virtual void OnClear(const glm::vec4& clearColor) = 0;

	protected:
		Engine* m_pEngine;
		std::vector<ReservedChunk> m_ReservedChunks;
		std::vector<RowData> m_Rows;
		uint32_t m_Width;
		uint32_t m_Height;
		uint32_t m_AvailableHeight;
	};
}
