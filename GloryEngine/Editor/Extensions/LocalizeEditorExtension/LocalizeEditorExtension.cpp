#include "LocalizeEditorExtension.h"
#include "TextDatabaseImporter.h"
#include "TextDatabaseTumbnailGenerator.h"
#include "TextDatabaseEditor.h"

#include <TextDatabase.h>
#include <LocalizeModule.h>

#include <ObjectMenu.h>
#include <FileBrowser.h>
#include <EditorAssetDatabase.h>
#include <ObjectMenuCallbacks.h>
#include <Tumbnail.h>
#include <MenuBar.h>

EXTENSION_CPP(LocalizeEditorExtension)

namespace Glory::Editor
{
	static constexpr char* Shortcut_Window_TextDBEditor = "Open Text Database Editor";

	TextDatabaseImporter Importer;
	TextDatabaseEditor Editor;

	LocalizeEditorExtension::LocalizeEditorExtension()
	{
	}

	LocalizeEditorExtension::~LocalizeEditorExtension()
	{
	}

	void OnCreateTextDatabase(Object*, const ObjectMenuType&)
	{
		std::filesystem::path path = FileBrowserItem::GetCurrentPath();
		path = GetUnqiueFilePath(path.append("New State Machine.gtdb"));

		FileBrowser::BeginCreate(path.filename().replace_extension("").string(), "", [](std::filesystem::path& finalPath) {
			finalPath.replace_extension("gtdb");
			if (std::filesystem::exists(finalPath)) return;
			TextDatabase* pTextDB = new TextDatabase();
			EditorAssetDatabase::CreateAsset(pTextDB, finalPath.string());
			FileBrowserItem::GetSelectedFolder()->Refresh();
			FileBrowserItem::GetSelectedFolder()->SortChildren();
		});
	}

	void LocalizeEditorExtension::Initialize()
	{
		EditorApplication* pApp = EditorApplication::GetInstance();
		MainEditor& editor = pApp->GetMainEditor();

		Importer::Register(&Importer);
		Tumbnail::AddGenerator<TextDatabaseTumbnailGenerator>();
		ObjectMenu::AddMenuItem("Create/Text Database", OnCreateTextDatabase, ObjectMenuType::T_ContentBrowser | ObjectMenuType::T_Resource | ObjectMenuType::T_Folder);
		MenuBar::AddMenuItem("Window/Localize/Properties", [&editor]() { editor.GetWindow<TextDatabaseEditor>(); }, NULL, Shortcut_Window_TextDBEditor);

		EditorPlayer::RegisterLoopHandler(this);
	}

	const char* LocalizeEditorExtension::ModuleName()
	{
		return "Localize";
	}

	void LocalizeEditorExtension::HandleBeforeStart(Module* pModule)
	{
	}

	void LocalizeEditorExtension::HandleStart(Module* pModule)
	{
	}

	void LocalizeEditorExtension::HandleStop(Module* pModule)
	{
	}

	void LocalizeEditorExtension::HandleUpdate(Module* pModule)
	{
	}
}
