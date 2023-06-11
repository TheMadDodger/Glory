#pragma once
#include "EntityPrefabData.h"

#include <PrefabDataLoader.h>
#include <NodeRef.h>

namespace Glory
{
	class EntityPrefabDataLoader : public PrefabDataLoader<EntityPrefabData, ImportSettings>
	{
	public:
		EntityPrefabDataLoader() : PrefabDataLoader(".gentity") {}
        virtual ~EntityPrefabDataLoader() = default;

    protected:
        virtual ImportSettings ReadImportSettings_Internal(YAML::Node& node) override { return {}; }
        virtual void WriteImportSettings_Internal(const ImportSettings& importSettings, YAML::Emitter& out) override {}

    private:
        virtual void Initialize() override {};
        virtual void Cleanup() override {};

    private:
        virtual EntityPrefabData* LoadResource(const std::string& path, const ImportSettings& importSettings) override;
        virtual EntityPrefabData* LoadResource(const void* buffer, size_t length, const ImportSettings& importSettings) override;
        virtual void SaveResource(const std::string& path, EntityPrefabData* pResource) override;

        void WriteChild(YAML::Emitter& out, const PrefabNode& parent);
        void ReadChild(EntityPrefabData* pPrefab, NodeValueRef nodeValue, PrefabNode& node);
	};
}
