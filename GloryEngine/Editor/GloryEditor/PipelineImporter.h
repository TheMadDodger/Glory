#pragma once
#include "ImporterTemplate.h"

#include <PipelineData.h>

namespace Glory::Editor
{
	/** @brief Pipeline importer */
    class PipelineImporter : public YAMLImporterTemplate<PipelineData>
    {
	public:
		/** @brief Constructor */
		PipelineImporter();
		/** @brief Destructor */
		virtual ~PipelineImporter();

		/** @brief Name of the importer
		 * @returns "Internal Pipeline Importer"
		 */
		std::string_view Name() const override;

	private:
		virtual bool SupportsExtension(const std::filesystem::path& extension) const override;
		virtual ImportedResource LoadResource(const std::filesystem::path& path, void*) const override;
		bool SaveResource(const std::filesystem::path& path, PipelineData* pResource) const override;

	private:
		virtual void Initialize() override;
		virtual void Cleanup() override;
    };
}
