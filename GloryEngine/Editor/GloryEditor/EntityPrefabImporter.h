#pragma once
#include <ImporterTemplate.h>
#include <PrefabData.h>
#include <NodeRef.h>

namespace Glory::Editor
{
	class EntityPrefabImporter : public YAMLImporterTemplate<PrefabData>
	{
	public:
		EntityPrefabImporter();
		virtual ~EntityPrefabImporter();

		std::string_view Name() const override;

	private:
		virtual bool SupportsExtension(const std::filesystem::path& extension) const override;
		virtual ImportedResource LoadResource(const std::filesystem::path& path, void*) const override;
		virtual bool SaveResource(const std::filesystem::path& path, PrefabData* pResource) const override;

	private:
		virtual void Initialize() override;
		virtual void Cleanup() override;
	};
}
