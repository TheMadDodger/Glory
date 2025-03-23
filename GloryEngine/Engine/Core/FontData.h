#pragma once
#include "Glory.h"
#include "FileData.h"
#include "FontDataStructs.h"

namespace Glory
{
    class InternalTexture;

    class FontData : public Resource
    {
    public:
        GLORY_API FontData();
        GLORY_API FontData(uint32_t height, std::vector<uint64_t>&& characterCodes,
            std::vector<GlyphData>&& chars, InternalTexture* pTexture);
        GLORY_API virtual ~FontData();

        GLORY_API uint32_t FontHeight() const;
        GLORY_API size_t GetGlyphIndex(uint64_t c) const;
        GLORY_API const GlyphData* GetGlyph(size_t index) const;
        GLORY_API InternalTexture* GetGlyphTexture() const;

        GLORY_API void Serialize(BinaryStream& container) const override;
        GLORY_API void Deserialize(BinaryStream& container) override;

    private:
        virtual void References(Engine*, std::vector<UUID>&) const override {}

    private:
        uint32_t m_FontHeight;
        std::vector<uint64_t> m_CharacterCodes;
        std::vector<GlyphData> m_Glyphs;
        InternalTexture* m_pTexture;
    };
}
