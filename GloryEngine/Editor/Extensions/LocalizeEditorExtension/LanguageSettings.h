#pragma once
#include <ProjectSettings.h>

namespace Glory::Editor
{
	class LanguageSettings : public ProjectSettings
	{
	public:
		LanguageSettings();

	private:
		virtual bool OnGui() override;
		virtual void OnSettingsLoaded() override;
		virtual void OnStartPlay_Impl() override;
		virtual void OnCompile(const std::filesystem::path& path) override;
	};
}
