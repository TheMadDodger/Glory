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
		for (size_t i = 0; i < m_Rows.size(); ++i)
		{
			RowData& row = m_Rows[i];
			if (row.Height != height) continue;
			/* Use this row if enough space is available */
			if (row.AvailableWidth < width)
			{
				/* Check if there is a gap */
				if (row.m_FreeGaps.empty()) continue;
				for (size_t j = 0; j < row.m_FreeGaps.size(); ++j)
				{
					auto& gap = row.m_FreeGaps[j];
					if (gap.first != width) continue;
					/* Get the xoffset */
					const uint32_t xoffset = m_Width - gap.first;
					/* Remove the gap */
					row.m_FreeGaps.erase(row.m_FreeGaps.begin() + j);
					/* Reserve the chunk */
					m_ReservedChunks.emplace_back(ReservedChunk{ id, xoffset, row.YOffset, width, height, uint32_t(i) });
					break;
				}
				continue;
			}
			/* Get the xoffset */
			const uint32_t xoffset = m_Width - row.AvailableWidth;
			/* Reduce available width for this row */
			row.AvailableWidth -= width;
			/* Reserve the chunk */
			m_ReservedChunks.emplace_back(ReservedChunk{ id, xoffset, row.YOffset, width, height, uint32_t(i) });
			return id;
		}

		/* No row was found, make a new row if space is available */
		if (m_AvailableHeight < height) return 0;
		/* Get yoffset for new row */
		const uint32_t yoffset = m_Height - m_AvailableHeight;
		/* Reduce available height */
		m_AvailableHeight -= height;
		/* Create the row */
		const uint32_t rowIndex = uint32_t(m_Rows.size());
		m_Rows.emplace_back(RowData{ yoffset, m_Width - width, height });
		/* Reserve the chunk */
		m_ReservedChunks.emplace_back(ReservedChunk{ id, 0, yoffset, width, height, rowIndex });
		return id;
	}

	bool TextureAtlas::HasReservedChunk(UUID id) const
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

	glm::vec4 TextureAtlas::GetChunkCoords(UUID id) const
	{
		auto& iter = std::find_if(m_ReservedChunks.begin(), m_ReservedChunks.end(), [id](const ReservedChunk& chunk) {
			return chunk.ID == id;
		});

		if (iter == m_ReservedChunks.end())
		{
			m_pEngine->GetDebug().LogError("TextureAtlas::GetChunkCoords() > Chunk not found!");
			return {};
		}

		const ReservedChunk& chunk = *iter;
		glm::vec4 coords;
		coords.x = float(chunk.XOffset)/m_Width;
		coords.y = float(chunk.YOffset)/m_Height;
		coords.z = float(chunk.XOffset + chunk.Width)/m_Width;
		coords.w = float(chunk.YOffset + chunk.Height)/m_Height;
		return coords;
	}

	void TextureAtlas::ReleaseChunk(UUID id)
	{
		auto iter = std::find_if(m_ReservedChunks.begin(), m_ReservedChunks.end(), [id](const ReservedChunk& chunk) { return chunk.ID == id; });
		if (iter == m_ReservedChunks.end()) return;
		RowData& row = m_Rows[iter->RowIndex];
		if (m_Width - iter->Width - row.AvailableWidth == iter->XOffset)
		{
			/* Just free a spot at the end */
			row.AvailableWidth += iter->Width;
			return;
		}

		/* We have to make a gap */
		row.m_FreeGaps.emplace_back(iter->XOffset, iter->Width);
	}

	void TextureAtlas::ReleaseAllChunks()
	{
		m_ReservedChunks.clear();
		m_Rows.clear();
	}
}
