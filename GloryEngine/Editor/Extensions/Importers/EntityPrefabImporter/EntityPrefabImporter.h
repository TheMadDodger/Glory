#pragma once
#include <ImporterTemplate.h>
#include <EntityPrefabData.h>
#include <NodeRef.h>

namespace Glory::Editor
{
	class EntityPrefabImporter : public ImporterTemplate<EntityPrefabData>
	{
	public:
		EntityPrefabImporter();
		virtual ~EntityPrefabImporter();

		std::string_view Name() const override;

	private:
		virtual bool SupportsExtension(const std::filesystem::path& extension) const override;
		virtual EntityPrefabData* LoadResource(const std::filesystem::path& path) const override;
		virtual bool SaveResource(const std::filesystem::path& path, EntityPrefabData* pResource) const override;

		void WriteChild(YAML::Emitter& out, const PrefabNode& parent) const;
		void ReadChild(EntityPrefabData* pPrefab, Utils::NodeValueRef nodeValue, PrefabNode& node) const;

	private:
		virtual void Initialize() override;
		virtual void Cleanup() override;
	};
}
