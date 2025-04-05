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
			finalPath.replace_extension("gui");
			if (std::filesystem::exists(finalPath)) return;
			FSMData* pFSM = new FSMData();
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
}
