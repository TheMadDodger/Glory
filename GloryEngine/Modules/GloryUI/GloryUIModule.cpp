#include "GloryUIModule.h"
#include "UIComponents.h"
#include "UISystems.h"
#include "FontData.h"

#include <Engine.h>
#include <RendererModule.h>
#include <SceneManager.h>
#include <Debug.h>
#include <ImageData.h>
#include <TextureData.h>
#include <AssetManager.h>

#include <EntityRegistry.h>

namespace Glory
{
	GLORY_MODULE_VERSION_CPP(GloryUIModule);

	GloryUIModule::GloryUIModule(): m_FT(nullptr) {}
	GloryUIModule::~GloryUIModule() {}

	const std::type_info& GloryUIModule::GetModuleType()
	{
		return typeid(GloryUIModule);
	}

	void GloryUIModule::CreateFontData(FontData* pFont)
	{
		Debug& debug = m_pEngine->GetDebug();

		auto iter = m_FontFaces.find(pFont->GetUUID());
		if (iter != m_FontFaces.end()) return;

		FT_Face face;
		if (FT_New_Memory_Face(m_FT, reinterpret_cast<const FT_Byte*>(pFont->Data()), pFont->Size(), 0, &face))
		{
			debug.LogError("FREETYPE: Failed to load font from memory");
			return;
		}

		FT_Set_Pixel_Sizes(face, 0, 48);

		std::vector<Character> characters;
		for (unsigned char c = 0; c < 128; c++)
		{
			// load character glyph 
			if (FT_Load_Char(face, c, FT_LOAD_RENDER))
			{
				debug.LogError("FREETYTPE: Failed to load Glyph");
				continue;
			}

			const size_t dataSize = face->glyph->bitmap.width*face->glyph->bitmap.rows;
			char* pixels = new char[dataSize];
			std::memcpy(pixels, face->glyph->bitmap.buffer, dataSize);

			ImageData* pImageData = new ImageData(face->glyph->bitmap.width, face->glyph->bitmap.rows,
				PixelFormat::PF_R, PixelFormat::PF_R, 1, std::move(pixels), face->glyph->bitmap.width*face->glyph->bitmap.rows);

			m_pEngine->GetAssetManager().AddLoadedResource(pImageData);
			TextureData* pTexture = new TextureData(pImageData);
			m_pEngine->GetAssetManager().AddLoadedResource(pTexture);

			Character character = {
				pTexture->GetUUID(),
				glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
				glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
				face->glyph->advance.x
			};

			characters.emplace_back(character);
		}
		m_FontFaces.emplace(pFont->GetUUID(), face);
		m_FontMaps.emplace(pFont->GetUUID(), std::move(characters));
	}

	Character* GloryUIModule::GetCharacterMap(FontData* pFont, uint32_t size)
	{
		auto iter = m_FontMaps.find(pFont->GetUUID());
		return iter == m_FontMaps.end() ? nullptr : &iter->second[0];
	}

	void GloryUIModule::Initialize()
	{
		Debug& debug = m_pEngine->GetDebug();

		if (FT_Init_FreeType(&m_FT) || !m_FT)
		{
			debug.LogFatalError("FREETYPE: Could not init FreeType Library");
			return;
		}

		std::stringstream str;
		FT_Int major, minor, build;
		FT_Library_Version(m_FT, &major, &minor, &build);
		str << "FREETYPE: Loaded freetype " << major << "." << minor << "." << build;
		debug.LogInfo(str.str());

		m_pEngine->GetResourceTypes().RegisterResource<FontData>("");

		Reflect::SetReflectInstance(&m_pEngine->Reflection());
		Reflect::RegisterType<TextComponent>();

		SceneManager* pScenes = m_pEngine->GetSceneManager();
		Glory::Utils::ECS::ComponentTypes* pComponentTypes = pScenes->ComponentTypesInstance();

		pComponentTypes->RegisterComponent<TextComponent>();

		/* Text Renderer */
		pComponentTypes->RegisterInvokaction<TextComponent>(Glory::Utils::ECS::InvocationType::Start, TextSystem::OnStart);
		pComponentTypes->RegisterInvokaction<TextComponent>(Glory::Utils::ECS::InvocationType::Stop, TextSystem::OnStop);
		pComponentTypes->RegisterInvokaction<TextComponent>(Glory::Utils::ECS::InvocationType::OnRemove, TextSystem::OnStop);
		pComponentTypes->RegisterInvokaction<TextComponent>(Glory::Utils::ECS::InvocationType::OnValidate, TextSystem::OnValidate);
		pComponentTypes->RegisterInvokaction<TextComponent>(Glory::Utils::ECS::InvocationType::Update, TextSystem::OnUpdate);
		pComponentTypes->RegisterInvokaction<TextComponent>(Glory::Utils::ECS::InvocationType::Draw, TextSystem::OnDraw);

		RendererModule* pRenderer = m_pEngine->GetMainModule<RendererModule>();
		pRenderer->SetFontGenerator(this);
	}

	void GloryUIModule::Cleanup()
	{
		FT_Done_FreeType(m_FT);
	}

	void GloryUIModule::GenerateFontImages(UUID id, FT_Face face, uint32_t size)
	{
		Debug& debug = m_pEngine->GetDebug();

		auto iter = m_FontMaps.find(id);
		if (iter == m_FontMaps.end()) return;

		FT_Set_Pixel_Sizes(face, 0, size);

		for (unsigned char c = 0; c < 128; c++)
		{
			// load character glyph 
			if (FT_Load_Char(face, c, FT_LOAD_RENDER))
			{
				debug.LogError("FREETYTPE: Failed to load Glyph");
				continue;
			}

			/*const size_t dataSize = face->glyph->bitmap.width*face->glyph->bitmap.rows;
			char* pixels = new char[dataSize];
			std::memcpy(pixels, face->glyph->bitmap.buffer, dataSize);*/

			//ImageData* pImageData = new ImageData(face->glyph->bitmap.width, face->glyph->bitmap.rows,
				//PixelFormat::PF_R, PixelFormat::PF_R8Uint, 1, std::move(pixels), face->glyph->bitmap.width * face->glyph->bitmap.rows);

			//m_pEngine->GetAssetManager().AddLoadedResource(pImageData);
			//TextureData* m_pTexture = new TextureData(pImageData);
			//m_pEngine->GetAssetManager().AddLoadedResource(m_pTexture);

			Character character = {
				0,//m_pTexture->GetUUID(),
				glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
				glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
				face->glyph->advance.x
			};

			iter->second.emplace_back(character);
		}
	}
}
