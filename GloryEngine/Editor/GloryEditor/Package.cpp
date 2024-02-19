#include "Package.h"

#include <GScene.h>
#include <AssetArchive.h>
#include <BinaryStream.h>

#include <filesystem>

namespace Glory
{
    void PackageScene(GScene* pScene, const std::filesystem::path& path)
    {
        BinaryFileStream file{ path };
        AssetArchive archive{&file};
        archive.Serialize(pScene);
    }
}
