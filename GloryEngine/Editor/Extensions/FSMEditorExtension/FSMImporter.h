#pragma once
#include <ImporterTemplate.h>
#include <FSM.h>
#include <Glory.h>

namespace Glory::Editor
{
    class FSMImporter : public YAMLImporterTemplate<FSMData>
    {
	public:
		FSMImporter() {};
		virtual ~FSMImporter() {};

		std::string_view Name() const override;

	private:
		virtual bool SupportsExtension(const std::filesystem::path& extension) const override;
		virtual ImportedResource LoadResource(const std::filesystem::path& path, void*) const override;
		virtual bool SaveResource(const std::filesystem::path& path, FSMData* pFSM) const override;

	private:
		virtual void Initialize() override;
		virtual void Cleanup() override {};
    };
}
