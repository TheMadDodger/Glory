#pragma once
#include "Texture.h"

namespace Glory
{
	class Texture;
	class Engine;

	class TextureAtlas
	{
	public:
		TextureAtlas(Engine* pEngine, uint32_t width, uint32_t height);
		virtual ~TextureAtlas();

		virtual void Initialize() = 0;
		virtual Texture* GetTexture() = 0;

		UUID ReserveChunk(uint32_t width, uint32_t height, UUID id=UUID());
		bool HasReservedChunk(UUID id);
		bool AsignChunk(UUID id, Texture* pTexture);

	protected:
		struct ReservedChunk
		{
			UUID ID;
			uint32_t XOffset;
			uint32_t YOffset;
			uint32_t Width;
			uint32_t Height;
		};

		struct RowData
		{
			uint32_t YOffset;
			uint32_t AvailableWidth;
			uint32_t Height;

			/* Keep track of gaps? */
		};

		virtual bool AssignChunk(Texture* pTexture, const ReservedChunk& chunk) = 0;

	protected:
		Engine* m_pEngine;
		std::vector<ReservedChunk> m_ReservedChunks;
		std::vector<RowData> m_Rows;
		uint32_t m_Width;
		uint32_t m_Height;
		uint32_t m_AvailableHeight;
	};
}
