#include "FSMEditorExtension.h"
#include "FSMImporter.h"
#include "FSMTumbnailGenerator.h"

#include <FSM.h>
#include <FSMModule.h>

#include <ObjectMenu.h>
#include <FileBrowser.h>
#include <EditorAssetDatabase.h>
#include <ObjectMenuCallbacks.h>
#include <Tumbnail.h>

#include <IconsFontAwesome6.h>
#include <PropertyDrawer.h>

EXTENSION_CPP(FSMEditorExtension)

namespace Glory::Editor
{
	FSMImporter importer;

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

			UUID node1;
			UUID node2;
			UUID node3;
			UUID node4;

			pFSM->NewNode("State 1", node1);
			pFSM->NewNode("State 2", node2);
			pFSM->NewNode("State 3", node3);
			pFSM->NewNode("State 4", node4);

			pFSM->NewTransition("Transition 1", node1, node2);
			pFSM->NewTransition("Transition 2", node1, node3);
			pFSM->NewTransition("Transition 3", node2, node3);
			pFSM->NewTransition("Transition 4", node3, node4);
			pFSM->NewTransition("Transition 5", node4, node1);

			EditorAssetDatabase::CreateAsset(pFSM, finalPath.string());
			FileBrowserItem::GetSelectedFolder()->Refresh();
			FileBrowserItem::GetSelectedFolder()->SortChildren();
		});
	}

	void FSMEditorExtension::Initialize()
	{
		Importer::Register(&importer);
		Tumbnail::AddGenerator<FSMTumbnailGenerator>();
		ObjectMenu::AddMenuItem("Create/Finite State Machine", OnCreateFSM, ObjectMenuType::T_ContentBrowser | ObjectMenuType::T_Resource | ObjectMenuType::T_Folder);
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
