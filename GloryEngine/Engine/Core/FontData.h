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
        GLORY_API FontData(std::vector<Character>&& chars, std::vector<InternalTexture*>&& textures);
        GLORY_API virtual ~FontData();

        GLORY_API const Character* GetCharacter(char c) const;
        GLORY_API InternalTexture* GetCharacterTexture(char c) const;

        GLORY_API void Serialize(BinaryStream& container) const override;
        GLORY_API void Deserialize(BinaryStream& container) override;

    private:
        std::vector<Character> m_Characters;
        std::vector<InternalTexture*> m_Textures;
    };
}
