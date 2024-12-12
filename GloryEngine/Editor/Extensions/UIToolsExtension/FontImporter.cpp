#include "FontImporter.h"

#include <EditorApplication.h>
#include <Engine.h>
#include <Debug.h>
#include <FileLoaderModule.h>
#include <InternalTexture.h>

#include <glm/ext/vector_int2.hpp>
#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

namespace Glory::Editor
{
	FT_Library FTLib;

	std::string_view FontImporter::Name() const
	{
		return "Font Importer";
	}

	bool FontImporter::SupportsExtension(const std::filesystem::path& extension) const
	{
		return extension.compare(".otf") == 0 || extension.compare(".ttf") == 0;
	}

	ImportedResource FontImporter::LoadResource(const std::filesystem::path& path, void*) const
	{
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		Debug& debug = pEngine->GetDebug();
		LoaderModule* pModule = pEngine->GetLoaderModule<FileData>();
		FileImportSettings fileImportSettings;
		fileImportSettings.m_Extension = "ttf";
		fileImportSettings.Flags = std::ios::binary | std::ios::ate;
		FileData* pFileData = (FileData*)pModule->Load(path.string(), fileImportSettings);

		FT_Face face;
		if (FT_New_Memory_Face(FTLib, reinterpret_cast<const FT_Byte*>(pFileData->Data()), pFileData->Size(), 0, &face))
		{
			debug.LogError("FREETYPE: Failed to load font from memory");
			return {};
		}

		const uint32_t fontHeight = 48;

		FT_Set_Pixel_Sizes(face, 0, fontHeight);

		FT_ULong charcode;
		FT_UInt gid;
		charcode = FT_Get_First_Char(face, &gid);

		std::vector<uint64_t> characterCodes;
		std::vector<GlyphData> glyphs;
		std::vector<InternalTexture*> textures;
		while (gid != 0)
		{
			// load character glyph 
			if (FT_Load_Char(face, charcode, FT_LOAD_RENDER))
			{
				debug.LogError("FREETYTPE: Failed to load Glyph");
				continue;
			}

			const size_t dataSize = face->glyph->bitmap.width * face->glyph->bitmap.rows;
			char* pixels = new char[dataSize];
			std::memcpy(pixels, face->glyph->bitmap.buffer, dataSize);

			ImageData* pImageData = new ImageData(face->glyph->bitmap.width, face->glyph->bitmap.rows,
				PixelFormat::PF_R, PixelFormat::PF_R, 1, std::move(pixels), face->glyph->bitmap.width * face->glyph->bitmap.rows);
			InternalTexture* pTexture = new InternalTexture(pImageData);

			GlyphData character = {
				glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
				glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
				face->glyph->advance.x
			};

			characterCodes.emplace_back(charcode);
			glyphs.emplace_back(character);
			textures.emplace_back(pTexture);

			charcode = FT_Get_Next_Char(face, charcode, &gid);
		}

		FontData* pFont = new FontData(fontHeight, std::move(characterCodes), std::move(glyphs), std::move(textures));
		delete pFileData;

		return { path, pFont };
	}

	void FontImporter::Initialize()
	{
		Debug& debug = EditorApplication::GetInstance()->GetEngine()->GetDebug();

		if (FT_Init_FreeType(&FTLib) || !FTLib)
		{
			debug.LogFatalError("FREETYPE: Could not init FreeType Library");
			return;
		}

		std::stringstream str;
		FT_Int major, minor, build;
		FT_Library_Version(FTLib, &major, &minor, &build);
		str << "FREETYPE: Loaded freetype " << major << "." << minor << "." << build;
		debug.LogInfo(str.str());
	}

	void FontImporter::Cleanup()
	{
		FT_Done_FreeType(FTLib);
	}
}
