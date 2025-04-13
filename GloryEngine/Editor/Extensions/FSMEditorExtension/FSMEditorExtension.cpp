#include "FSMEditorExtension.h"
#include "FSMImporter.h"
#include "FSMTumbnailGenerator.h"
#include "FSMEditor.h"
#include "FSMPropertiesWindow.h"
#include "FSMNodeEditor.h"
#include "FSMNodeInspector.h"
#include "FSMDebugger.h"

#include <FSM.h>
#include <FSMModule.h>

#include <ObjectMenu.h>
#include <FileBrowser.h>
#include <EditorAssetDatabase.h>
#include <ObjectMenuCallbacks.h>
#include <Tumbnail.h>
#include <MenuBar.h>

#include <IconsFontAwesome6.h>
#include <PropertyDrawer.h>

EXTENSION_CPP(FSMEditorExtension)

namespace Glory::Editor
{
	static constexpr char* Shortcut_Window_FSMProperties = "Open FSM Properties";
	static constexpr char* Shortcut_Window_FSMNodes = "Open FSM Node Editor";
	static constexpr char* Shortcut_Window_FSMInspector = "Open FSM Inspector";
	static constexpr char* Shortcut_Window_FSMDebugger = "Open FSM Debugger";

	FSMImporter Importer;
	FSMEditor Editor;

	FSMEditorExtension::FSMEditorExtension()
	{
	}

	FSMEditorExtension::~FSMEditorExtension()
	{
	}

	void OnCreateFSM(Object*, const ObjectMenuType&)
	{
		std::filesystem::path path = FileBrowserItem::GetCurrentPath();
		path = GetUnqiueFilePath(path.append("New State Machine.gfsm"));

		FileBrowser::BeginCreate(path.filename().replace_extension("").string(), "", [](std::filesystem::path& finalPath) {
			finalPath.replace_extension("gfsm");
			if (std::filesystem::exists(finalPath)) return;
			FSMData* pFSM = new FSMData();
			EditorAssetDatabase::CreateAsset(pFSM, finalPath.string());
			FileBrowserItem::GetSelectedFolder()->Refresh();
			FileBrowserItem::GetSelectedFolder()->SortChildren();
		});
	}

	void FSMEditorExtension::Initialize()
	{
		EditorApplication* pApp = EditorApplication::GetInstance();
		MainEditor& editor = pApp->GetMainEditor();
		pApp->GetMainEditor().RegisterMainWindow(&Editor);

		Importer::Register(&Importer);
		Tumbnail::AddGenerator<FSMTumbnailGenerator>();
		ObjectMenu::AddMenuItem("Create/Finite State Machine", OnCreateFSM, ObjectMenuType::T_ContentBrowser | ObjectMenuType::T_Resource | ObjectMenuType::T_Folder);
		MenuBar::AddMenuItem("Window/FSM Editor/Properties", [&editor]() { editor.GetWindow<FSMEditor, FSMPropertiesWindow>(); }, NULL, Shortcut_Window_FSMProperties);
		MenuBar::AddMenuItem("Window/FSM Editor/Nodes", [&editor]() { editor.GetWindow<FSMEditor, FSMNodeEditor>(); }, NULL, Shortcut_Window_FSMNodes);
		MenuBar::AddMenuItem("Window/FSM Editor/Inspector", [&editor]() { editor.GetWindow<FSMEditor, FSMNodeInspector>(); }, NULL, Shortcut_Window_FSMInspector);
		MenuBar::AddMenuItem("Window/FSM Editor/Debugger", [&editor]() { editor.GetWindow<FSMEditor, FSMDebugger>(); }, NULL, Shortcut_Window_FSMDebugger);

		EditorPlayer::RegisterLoopHandler(this);

		FSMEditor* pMainWindow = editor.FindMainWindow<FSMEditor>();
		pMainWindow->GetWindow<FSMPropertiesWindow>();
		pMainWindow->GetWindow<FSMNodeEditor>();
		pMainWindow->GetWindow<FSMNodeInspector>();
		pMainWindow->GetWindow<FSMDebugger>();
	}

	const char* FSMEditorExtension::ModuleName()
	{
		return "Finite State Machines";
	}

	void FSMEditorExtension::HandleBeforeStart(Module* pModule)
	{
	}

	void FSMEditorExtension::HandleStart(Module* pModule)
	{
	}

	void FSMEditorExtension::HandleStop(Module* pModule)
	{
		FSMModule* pFSMModule = static_cast<FSMModule*>(pModule);
		pFSMModule->CleanupStates();
	}

	void FSMEditorExtension::HandleUpdate(Module* pModule)
	{
	}
}
