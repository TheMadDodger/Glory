#include "StringsOverrideTableEditor.h"
#include "StringTableEditor.h"

#include <AssetPicker.h>
#include <EditorUI.h>
#include <Undo.h>
#include <ProjectSettings.h>
#include <MainEditor.h>
#include <EditorApplication.h>

namespace Glory::Editor
{
	StringsOverrideTableEditor::StringsOverrideTableEditor()
	{
	}

	StringsOverrideTableEditor::~StringsOverrideTableEditor()
	{
	}

	void StringsOverrideTableEditor::Initialize()
	{
	}

	bool StringsOverrideTableEditor::OnGUI()
	{
		YAMLResource<StringsOverrideTable>* pTableData = (YAMLResource<StringsOverrideTable>*)m_pTarget;
		Utils::YAMLFileRef& file = **pTableData;
		auto baseTable = file["BaseTable"];
		const UUID baseTableID = baseTable.As<uint64_t>(0);
		UUID newBaseTableID = baseTableID;
		bool change = false;
		const uint32_t stringTableHash = ResourceTypes::GetHash<StringTable>();
		if (AssetPicker::ResourceDropdown("Base Table", stringTableHash, &newBaseTableID, false))
		{
			Undo::StartRecord("Base Table Change", pTableData->GetUUID());
			Undo::ApplyYAMLEdit(file, baseTable.Path(), uint64_t(baseTableID), uint64_t(newBaseTableID));
			Undo::StopRecord();
			change = true;
		}
		auto language = file["Language"];
		ProjectSettings* pLanguageSettings = ProjectSettings::Get("Languages");
		Utils::YAMLFileRef& languagesFile = **pLanguageSettings;
		auto defaultLang = languagesFile["DefaultLanguage"];
		auto supportedLanguages = languagesFile["SupportedLanguages"];

		size_t index = 0;
		std::vector<std::string> languages;
		std::vector<std::string_view> languageViews;
		const std::string& defaultLanguage = languages.emplace_back(std::move(defaultLang.As<std::string>()));
		if (!language.Exists()) language.Set(defaultLanguage);
		const std::string currentLanguage = language.As<std::string>();

		for (size_t i = 0; i < supportedLanguages.Size(); ++i)
		{
			const std::string& language = languages.emplace_back(std::move(supportedLanguages[i].As<std::string>()));
			if (currentLanguage != language) continue;
			index = i + 1;
		}

		for (size_t i = 0; i < languages.size(); ++i)
			languageViews.emplace_back(languages[i]);

		if (EditorUI::InputDropdown("Language", languageViews, &index, currentLanguage))
		{
			Undo::StartRecord("Language Change", pTableData->GetUUID());
			Undo::ApplyYAMLEdit(file, language.Path(), currentLanguage, languages[index]);
			Undo::StopRecord();
		}

		if (ImGui::Button("Edit Table", { ImGui::GetContentRegionAvail().x, 0.0f }))
		{
			MainEditor& editor = EditorApplication::GetInstance()->GetMainEditor();
			StringTableEditor* pEditor = editor.GetWindow<StringTableEditor>();
			pEditor->SetTable(pTableData->GetUUID());
		}
		return change;
	}

	std::string StringsOverrideTableEditor::Name()
	{
		return "Strings Override Table";
	}
}
