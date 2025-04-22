#include "LanguageSettings.h"
#include "EditorUI.h"
#include "EditorApplication.h"
#include "Undo.h"

#include <LocalizeModule.h>

#include <Engine.h>
#include <LayerManager.h>
#include <imgui.h>
#include <BinaryStream.h>
#include <ListView.h>

namespace Glory::Editor
{
	LanguageSettings::LanguageSettings() : ProjectSettings("Languages") {}

	bool LanguageSettings::OnGui()
	{
		auto defaultLang = m_YAMLFile["DefaultLanguage"];
		bool change = EditorUI::InputText(m_YAMLFile, "DefaultLanguage");

		ListView listView = ListView("Supported Languages");

		auto supportedLanguages = m_YAMLFile["SupportedLanguages"];
		if (!supportedLanguages.IsSequence()) supportedLanguages.SetSequence();
		listView.OnDrawElement = [&](size_t index) {
			Utils::NodeValueRef supportedLang = supportedLanguages[index];
			change |= EditorUI::InputText(m_YAMLFile, supportedLang.Path());
		};

		listView.OnAdd = [&]() {
			YAML::Node newNode{ YAML::NodeType::Scalar };
			newNode = "New Language";

			const size_t count = supportedLanguages.Size();
			Undo::StartRecord("Add Language");
			YAML::Node oldValue = YAML::Node(YAML::NodeType::Null);
			Utils::NodeValueRef supportedLang = supportedLanguages[count];
			Undo::YAMLEdit(m_YAMLFile, supportedLang.Path(), oldValue, newNode);
			Undo::StopRecord();

			change = true;
		};

		listView.OnRemove = [&](int index) {
			Undo::StartRecord("Remove Input Map");
			YAML::Node oldValue = supportedLanguages[index].Node();
			YAML::Node newValue = YAML::Node(YAML::NodeType::Null);
			Utils::NodeValueRef supportedLang = supportedLanguages[index];
			Undo::YAMLEdit(m_YAMLFile, supportedLang.Path(), oldValue, newValue);
			supportedLanguages.Remove(index);
			Undo::StopRecord();
			change = true;
		};

		listView.Draw(supportedLanguages.Size());

		return change;
	}

	void LanguageSettings::OnSettingsLoaded()
	{
		auto defaultLang = m_YAMLFile["DefaultLanguage"];
		if (!defaultLang.Exists()) defaultLang.Set("English");
		auto supportedLanguages = m_YAMLFile["SupportedLanguages"];
		if (!supportedLanguages.IsSequence()) supportedLanguages.SetSequence();
	}

	void LanguageSettings::OnStartPlay_Impl()
	{
		auto defaultLang = m_YAMLFile["DefaultLanguage"];
		if (!defaultLang.Exists()) defaultLang.Set("English");
		auto supportedLanguages = m_YAMLFile["SupportedLanguages"];
		if (!supportedLanguages.IsSequence()) supportedLanguages.SetSequence();

		LocalizeModule* pLocalize = EditorApplication::GetInstance()->GetEngine()->GetOptionalModule<LocalizeModule>();
		std::string defaultLanguage = defaultLang.As<std::string>();
		std::vector<std::string> supportedLangs;
		for (size_t i = 0; i < supportedLanguages.Size(); ++i)
		{
			supportedLangs.emplace_back(supportedLanguages[i].As<std::string>());
		}
		pLocalize->SetLanguages(std::move(defaultLanguage), std::move(supportedLangs));
	}

	void LanguageSettings::OnCompile(const std::filesystem::path& path)
	{
		auto defaultLang = m_YAMLFile["DefaultLanguage"];
		if (!defaultLang.Exists()) defaultLang.Set("English");
		auto supportedLanguages = m_YAMLFile["SupportedLanguages"];
		if (!supportedLanguages.IsSequence()) supportedLanguages.SetSequence();

		std::filesystem::path finalPath = path;
		finalPath.replace_filename("Languages.dat");
		BinaryFileStream file{ finalPath };
		BinaryStream* stream = &file;
		stream->Write(CoreVersion);

		stream->Write(defaultLang.As<std::string>());
		stream->Write(supportedLanguages.Size());
		for (size_t i = 0; i < supportedLanguages.Size(); ++i)
		{
			stream->Write(supportedLanguages[i].As<std::string>());
		}
	}
}
