#include "EntityPrefabDataLoader.h"

namespace Glory
{
    EntityPrefabData* EntityPrefabDataLoader::LoadResource(const std::string& path, const ImportSettings& importSettings)
    {
        return nullptr;
    }

    EntityPrefabData* EntityPrefabDataLoader::LoadResource(const void* buffer, size_t length, const ImportSettings& importSettings)
    {
        return nullptr;
    }

    void EntityPrefabDataLoader::SaveResource(const std::string& path, EntityPrefabData* pResource)
    {
    }
}
