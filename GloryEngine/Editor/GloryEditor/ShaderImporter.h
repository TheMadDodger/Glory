#pragma once
#include "ImporterTemplate.h"

#include <ShaderSourceData.h>

namespace Glory::Editor
{
	class ShaderImporter : public ImporterTemplate<ShaderSourceData>
	{
	public:
		ShaderImporter() {};
		virtual ~ShaderImporter() {};

		std::string_view Name() const override;

	private:
		virtual bool SupportsExtension(const std::filesystem::path& extension) const override;
		virtual ImportedResource LoadResource(const std::filesystem::path& path) const override;

	private:
		virtual void Initialize() override {};
		virtual void Cleanup() override {};
	};
}
