#pragma once
#include "GraphicsHandles.h"

#include <glm/vec4.hpp>
#include <glm/vec2.hpp>

namespace Glory
{
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
		virtual TextureHandle GetTexture() const = 0;

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
		/** @brief Get the texture coordinates of a chunk
		 * @param id ID of the chunk
		 */
		glm::vec4 GetChunkCoords(UUID id) const;
		glm::vec4 GetChunkPositionAndSize(UUID id) const;

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

		glm::uvec2 Resolution() const;

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
		/** @brief Implementation for resizing the atlas */
		virtual void OnResize() = 0;

	protected:
		Engine* m_pEngine;
		std::vector<ReservedChunk> m_ReservedChunks;
		std::vector<RowData> m_Rows;
		uint32_t m_Width;
		uint32_t m_Height;
		uint32_t m_AvailableHeight;
	};
}
