#pragma once
#include <ImporterTemplate.h>
#include <PrefabData.h>
#include <NodeRef.h>

namespace Glory::Editor
{
	class EntityPrefabImporter : public ImporterTemplate<PrefabData>
	{
	public:
		EntityPrefabImporter();
		virtual ~EntityPrefabImporter();

		std::string_view Name() const override;

	private:
		virtual bool SupportsExtension(const std::filesystem::path& extension) const override;
		virtual PrefabData* LoadResource(const std::filesystem::path& path) const override;
		virtual bool SaveResource(const std::filesystem::path& path, PrefabData* pResource) const override;

		void WriteChild(YAML::Emitter& out, const PrefabNode& parent) const;
		void ReadChild(PrefabData* pPrefab, Utils::NodeValueRef nodeValue, PrefabNode& node) const;

	private:
		virtual void Initialize() override;
		virtual void Cleanup() override;
	};
}
