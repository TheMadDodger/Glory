#include "ProjectSettings.h"
#include "EditorUI.h"
#include "Undo.h"
#include "EditorApplication.h"

#include <BinaryStream.h>
#include <Glory.h>
#include <Engine.h>

namespace Glory::Editor
{
	bool GeneralSettings::OnGui()
	{
		ImGui::TextUnformatted("Version");
		ImGui::Separator();

		auto root = RootValue();
		if (!root.IsMap()) root.SetMap();
		auto major = root["Major"];
		auto minor = root["Minor"];
		auto subMinor = root["SubMinor"];
		auto enableRC = root["EnableReleaseCanditate"];
		auto rc = root["RC"];

		if (!major.Exists()) major.Set(0u);
		if (!minor.Exists()) minor.Set(1u);
		if (!subMinor.Exists()) subMinor.Set(0u);
		if (!rc.Exists()) rc.Set(0u);
		if (!enableRC.Exists()) enableRC.Set(false);

		bool change = EditorUI::InputUInt(m_YAMLFile, major.Path());
		change |= EditorUI::InputUInt(m_YAMLFile, minor.Path());
		change |= EditorUI::InputUInt(m_YAMLFile, subMinor.Path());

		change |= EditorUI::CheckBox(m_YAMLFile, enableRC.Path());
		if (enableRC.As<bool>())
			change |= EditorUI::InputUInt(m_YAMLFile, rc.Path());
		else if(rc.As<uint32_t>() != 0u)
			rc.Set(0u);

		auto organization = root["Organization"];
		auto appName = root["ApplicationName"];
		ImGui::Separator();
		ImGui::TextUnformatted("Application");
		change |= EditorUI::InputText(m_YAMLFile, organization.Path());
		change |= EditorUI::InputText(m_YAMLFile, appName.Path());

		return change;
	}

	void GeneralSettings::OnSettingsLoaded()
	{
		auto root = RootValue();
		if (!root.IsMap()) root.SetMap();

		auto major = root["Major"];
		auto minor = root["Minor"];
		auto subMinor = root["SubMinor"];
		auto enableRC = root["EnableReleaseCanditate"];
		auto rc = root["RC"];
		auto organization = root["Organization"];
		auto appName = root["ApplicationName"];

		if (!major.Exists()) major.Set(0u);
		if (!minor.Exists()) minor.Set(1u);
		if (!subMinor.Exists()) subMinor.Set(0u);
		if (!rc.Exists()) rc.Set(0u);
		if (!enableRC.Exists()) enableRC.Set(false);
		if (!organization.Exists()) organization.Set("My Company");
		if (!appName.Exists()) appName.Set("My Game");

		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		pEngine->SetOrganizationAndAppName(std::move(organization.As<std::string>()), std::move(appName.As<std::string>()));
	}

	void GeneralSettings::OnCompile(const std::filesystem::path& path)
	{
		std::filesystem::path finalPath = path;
		finalPath.replace_filename("General.dat");
		BinaryFileStream file{ finalPath };
		BinaryStream* stream = &file;
		stream->Write(CoreVersion);
		
		auto root = RootValue();
		auto major = root["Major"];
		auto minor = root["Minor"];
		auto subMinor = root["SubMinor"];
		auto rc = root["RC"];

		Version version{ major.As<int>(0), minor.As<int>(1), subMinor.As<int>(0), rc.As<int>(0) };
		stream->Write(version);

		auto organization = root["Organization"];
		auto appName = root["ApplicationName"];
		stream->Write(organization.As<std::string>()).Write(appName.As<std::string>());
	}

	void GeneralSettings::OnStartPlay_Impl()
	{
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		auto root = RootValue();
		auto major = root["Major"];
		auto minor = root["Minor"];
		auto subMinor = root["SubMinor"];
		auto rc = root["RC"];
		pEngine->SetApplicationVersion(major.As<uint32_t>(0), minor.As<uint32_t>(1), subMinor.As<uint32_t>(0), rc.As<uint32_t>(0));

		auto organization = root["Organization"];
		auto appName = root["ApplicationName"];
		pEngine->SetOrganizationAndAppName(std::move(organization.As<std::string>()), std::move(appName.As<std::string>()));
	}
}
