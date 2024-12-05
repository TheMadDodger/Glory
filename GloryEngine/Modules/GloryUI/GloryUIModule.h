#pragma once
#include <Module.h>

#include <freetype2/ft2build.h>
#include <glm/ext/vector_int2.hpp>
#include FT_FREETYPE_H

namespace Glory
{
	class FontData;

	struct Character
	{
		UUID TextureID;
		glm::ivec2 Size;
		glm::ivec2 Bearing;
		uint32_t Advance;
	};

	struct CharacterSizeOffset
	{
		uint32_t Offset;
		uint32_t Length;
	};

	struct FontCharacterMaps
	{
		std::vector<std::pair<uint32_t, CharacterSizeOffset>> Offsets;
		std::vector<Character> Characters;
	};

	/** @brief SDL Audio Module using SDL_mixer */
    class GloryUIModule : public Module
    {
	public:
		/** @brief Constructor */
		GloryUIModule();
		/** @brief Destructor */
		virtual ~GloryUIModule();

		/** @brief GloryUIModule type */
		const std::type_info& GetModuleType() override;

		GLORY_MODULE_VERSION_H(0,1,0);

		FT_Face GetFontFace(FontData* pFont);

		void ReserveFontSize(FontData* pFont, uint32_t size);
		const Character& GetCharacter(FontData* pFont, uint32_t size, char c);

	protected:
		virtual void Initialize() override;
		virtual void Cleanup() override;

	private:
		CharacterSizeOffset GenerateFontImages(UUID id, FT_Face face, uint32_t size);

	private:
		FT_Library m_FT;
		std::map<UUID, FT_Face> m_FontFaces;
		std::map<UUID, FontCharacterMaps> m_FontMaps;
    };
}
