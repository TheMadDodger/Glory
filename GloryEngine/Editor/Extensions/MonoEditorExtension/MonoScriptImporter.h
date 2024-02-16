#pragma once
#include <ImporterTemplate.h>
#include <MonoScript.h>
#include <NodeRef.h>

namespace Glory::Editor
{
	class MonoScriptImporter : public ImporterTemplate<MonoScript>
	{
	public:
		MonoScriptImporter() {};
		virtual ~MonoScriptImporter() {};

		std::string_view Name() const override;

	private:
		virtual bool SupportsExtension(const std::filesystem::path& extension) const override;
		virtual ImportedResource LoadResource(const std::filesystem::path& path) const override;
		virtual bool SaveResource(const std::filesystem::path& path, MonoScript* pScript) const override;

	private:
		virtual void Initialize() override {};
		virtual void Cleanup() override {};
	};
}
