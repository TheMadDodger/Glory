#pragma once
#include <Glory.h>
#include <FileData.h>

namespace Glory
{
    class FontData : public FileData
    {
    public:
        GLORY_API FontData();
        GLORY_API FontData(FileData* pFileData);
        GLORY_API virtual ~FontData() {};

        GLORY_API void Serialize(BinaryStream& container) const override;
        GLORY_API void Deserialize(BinaryStream& container) override;
    };
}
