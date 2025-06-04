#include "TextureAtlas.h"
#include "Texture.h"
#include "Engine.h"
#include "Debug.h"
#include "RenderTexture.h"

namespace Glory
{
	TextureAtlas::TextureAtlas(Engine* pEngine, uint32_t width, uint32_t height):
		m_pEngine(pEngine), m_Width(width), m_Height(height), m_AvailableHeight(height)
	{
	}

	TextureAtlas::~TextureAtlas()
	{
	}

	UUID TextureAtlas::ReserveChunk(uint32_t width, uint32_t height, UUID id)
	{
		/* Find a row with the same height */
		for (RowData& row : m_Rows)
		{
			if (row.Height != height) continue;
			/* Use this row if enough space is available */
			if (row.AvailableWidth < width) continue;
			/* Get the xoffset */
			const uint32_t xoffset = m_Width - row.AvailableWidth;
			/* Reduce available width for this row */
			row.AvailableWidth -= width;
			/* Reserve the chunk */
			m_ReservedChunks.emplace_back(ReservedChunk{ id, xoffset, row.YOffset, width, height });
			return id;
		}

		/* No row was found, make a new row if space is available */
		if (m_AvailableHeight < height) return 0;
		/* Get yoffset for new row */
		const uint32_t yoffset = m_Height - m_AvailableHeight;
		/* Reduce available height */
		m_AvailableHeight -= height;
		/* Create the row */
		m_Rows.emplace_back(RowData{ yoffset, m_Width - width, height });
		/* Reserve the chunk */
		m_ReservedChunks.emplace_back(ReservedChunk{ id, 0, yoffset, width, height });
		return id;
	}

	bool TextureAtlas::HasReservedChunk(UUID id)
	{
		return std::find_if(m_ReservedChunks.begin(), m_ReservedChunks.end(),
				[id](const ReservedChunk& chunk) { return chunk.ID == id; })
			!= m_ReservedChunks.end();
	}

	bool TextureAtlas::AsignChunk(UUID id, Texture* pTexture)
	{
		auto& iter = std::find_if(m_ReservedChunks.begin(), m_ReservedChunks.end(), [id](const ReservedChunk& chunk) {
			return chunk.ID == id;
		});

		if (iter == m_ReservedChunks.end())
		{
			m_pEngine->GetDebug().LogError("TextureAtlas::AsignChunk(Texture) > Chunk not found!");
			return false;
		}

		const TextureCreateInfo& info = pTexture->Info();
		const ReservedChunk& chunk = *iter;
		if (info.m_Width != chunk.Width || info.m_Height != chunk.Height)
		{
			m_pEngine->GetDebug().LogError("TextureAtlas::AsignChunk(Texture) > Texture has incorrect size!");
			return false;
		}

		return AssignChunk(pTexture, chunk);
	}
}
