#pragma once
#include <Module.h>
#include <IFontImageGenerator.h>

#include <CameraRef.h>
#include <FontDataStructs.h>

#include <freetype2/ft2build.h>
#include <glm/ext/vector_int2.hpp>
#include FT_FREETYPE_H

namespace Glory
{
	class FontData;
	class MeshData;
	struct FontCharacterMaps;

	/** @brief SDL Audio Module using SDL_mixer */
    class GloryUIModule : public Module, public IFontImageGenerator
    {
	public:
		/** @brief Constructor */
		GloryUIModule();
		/** @brief Destructor */
		virtual ~GloryUIModule();

		/** @brief GloryUIModule type */
		const std::type_info& GetModuleType() override;

		GLORY_MODULE_VERSION_H(0,1,0);

		void CreateFontData(FontData* pFont);

		virtual Character* GetCharacterMap(FontData* pFont, uint32_t size) override;

	protected:
		virtual void Initialize() override;
		virtual void Cleanup() override;

	private:
		void GenerateFontImages(UUID id, FT_Face face, uint32_t size);

	private:
		FT_Library m_FT;
		std::map<UUID, FT_Face> m_FontFaces;
		std::map<UUID, std::vector<Character>> m_FontMaps;
    };
}
