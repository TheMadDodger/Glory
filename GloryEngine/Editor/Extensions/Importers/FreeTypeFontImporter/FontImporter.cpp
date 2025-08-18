#include "FontImporter.h"
#include "EditorPipelineManager.h"

#include <EditorApplication.h>
#include <Engine.h>
#include <Debug.h>
#include <FileLoaderModule.h>
#include <InternalTexture.h>
#include <MaterialData.h>
#include <PipelineData.h>

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

		uint32_t width = 1024, height = 1024;

		char* texturePixels = new char[width*height];
		std::memset(texturePixels, '\0', width * height);
		const uint32_t padding = 4;
		const uint32_t edgePadding = padding;
		uint32_t writeX = edgePadding, writeY = edgePadding;
		uint32_t highestGlyph = 0;

		std::vector<uint64_t> characterCodes;
		std::vector<GlyphData> glyphs;
		while (gid != 0)
		{
			// load character glyph 
			if (FT_Load_Char(face, charcode, FT_LOAD_RENDER))
			{
				debug.LogError("FREETYTPE: Failed to load Glyph");
				continue;
			}

			if (writeX + padding + face->glyph->bitmap.width >= width - edgePadding)
			{
				writeX = edgePadding;
				writeY += highestGlyph + padding;
				highestGlyph = 0;
			}
			for (size_t row = 0; row < face->glyph->bitmap.rows; ++row)
			{
				char* writeBuffer = &texturePixels[writeY*width + writeX + row*width];
				unsigned char* readBuffer = &face->glyph->bitmap.buffer[row*face->glyph->bitmap.width];
				std::memcpy(writeBuffer, readBuffer, face->glyph->bitmap.width);
				highestGlyph = std::max(highestGlyph, face->glyph->bitmap.rows);
			}

			const glm::vec4 texCoords{ float(writeX) / width, float(writeY) / height,
				float(writeX + face->glyph->bitmap.width) / width, float(writeY + face->glyph->bitmap.rows) / height };
			writeX += face->glyph->bitmap.width + padding;

			GlyphData character = {
				charcode,
				glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
				glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
				face->glyph->advance.x,
				texCoords
			};

			characterCodes.emplace_back(charcode);
			glyphs.emplace_back(character);
			charcode = FT_Get_Next_Char(face, charcode, &gid);
		}

		ImageData* pImageData = new ImageData(width, height,
			PixelFormat::PF_R, PixelFormat::PF_R, 1, std::move(texturePixels), width*height);

		FontData* pFont = new FontData(fontHeight, std::move(characterCodes), std::move(glyphs));
		ImportedResource resource = { path, pFont };
		resource.AddChild(pImageData, "Image");
		TextureData* pTexture = new TextureData(pImageData);
		SamplerSettings& sampler = pTexture->GetSamplerSettings();
		sampler.MipmapMode = Filter::F_None;
		resource.AddChild(pTexture, "Texture");
		pFont->SetTexture(pTexture->GetUUID());

		FT_Done_Face(face);
		delete pFileData;

		EditorPipelineManager& pipelines = EditorApplication::GetInstance()->GetPipelineManager();
		const UUID pipelineID = pipelines.FindPipeline(PipelineType::PT_Text, true);

		if (!pipelineID) return resource;
		PipelineData* pPipeline = pipelines.GetPipelineData(pipelineID);
		MaterialData* pDefaultMaterial = new MaterialData();
		pDefaultMaterial->SetPipeline(pipelineID);
		pPipeline->LoadIntoMaterial(pDefaultMaterial);
		resource.AddChild(pDefaultMaterial, "Material");
		pFont->SetMaterial(pDefaultMaterial->GetUUID());
		pDefaultMaterial->SetTexture("texSampler", pTexture->GetUUID());

		return resource;
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
