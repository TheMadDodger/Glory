#pragma once
#include "Glory.h"
#include "FileData.h"
#include "FontDataStructs.h"

namespace Glory
{
    class AssetManager;
    class TextureData;

    class FontData : public Resource
    {
    public:
        GLORY_API FontData();
        GLORY_API FontData(uint32_t height, std::vector<uint64_t>&& characterCodes,
            std::vector<GlyphData>&& chars);
        GLORY_API virtual ~FontData();

        GLORY_API uint32_t FontHeight() const;
        GLORY_API size_t GetGlyphIndex(uint64_t c) const;
        GLORY_API const GlyphData* GetGlyph(size_t index) const;
        GLORY_API TextureData* GetGlyphTexture(AssetManager& assets) const;
        GLORY_API void SetTexture(UUID texture);
        GLORY_API void SetMaterial(UUID material);
        GLORY_API UUID Texture();
        GLORY_API UUID Material();

        GLORY_API void Serialize(BinaryStream& container) const override;
        GLORY_API void Deserialize(BinaryStream& container) override;

    private:
        void References(Engine* pEngine, std::vector<UUID>& references) const override;

    private:
        uint32_t m_FontHeight;
        std::vector<uint64_t> m_CharacterCodes;
        std::vector<GlyphData> m_Glyphs;
        UUID m_Texture;
        UUID m_Material;
    };
}
