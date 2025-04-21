#include "LocalizeEditorExtension.h"
#include "StringTableImporter.h"
#include "StringTableTumbnailGenerator.h"
//#include "StringTableEditor.h"

#include <Localize.h>
#include <StringTable.h>
#include <LocalizeModule.h>
#include <AssetArchive.h>
#include <BinaryStream.h>
#include <AssetManager.h>
#include <SceneManager.h>

#include <ObjectMenu.h>
#include <FileBrowser.h>
#include <EditorAssetDatabase.h>
#include <ObjectMenuCallbacks.h>
#include <Tumbnail.h>
#include <MenuBar.h>
#include <Package.h>
#include <Dispatcher.h>
#include <EditorResourceManager.h>
#include <EntitySceneObjectEditor.h>
#include <CreateEntityObjectsCallbacks.h>
#include <EditorAssetCallbacks.h>
#include <CreateObjectAction.h>
#include <EditableEntity.h>
#include <EntityEditor.h>

#include <IconsFontAwesome6.h>

EXTENSION_CPP(LocalizeEditorExtension)

namespace Glory::Editor
{
	CREATE_OBJECT_CALLBACK_CPP(Localize, Localize, ());

	static constexpr char* Shortcut_Window_StringTableEditor = "Open String Table Editor";

	StringTableImporter Importer;
	//StringTableEditor Editor;

	LocalizeEditorExtension::LocalizeEditorExtension()
	{
	}

	LocalizeEditorExtension::~LocalizeEditorExtension()
	{
	}

	void OnCreateStringTable(Object*, const ObjectMenuType&)
	{
		std::filesystem::path path = FileBrowserItem::GetCurrentPath();
		path = GetUnqiueFilePath(path.append("New String Table.gtable"));

		FileBrowser::BeginCreate(path.filename().replace_extension("").string(), "", [](std::filesystem::path& finalPath) {
			finalPath.replace_extension("gtable");
			if (std::filesystem::exists(finalPath)) return;
			StringTable* pStringTable = new StringTable();
			EditorAssetDatabase::CreateAsset(pStringTable, finalPath.string());
			FileBrowserItem::GetSelectedFolder()->Refresh();
			FileBrowserItem::GetSelectedFolder()->SortChildren();
		});
	}

	void PackageStringTables(Glory::Engine* pEngine, const std::filesystem::path& packageRoot, PackageTaskState& task)
	{
		std::filesystem::path localePath = packageRoot;
		localePath.append("Data/Locale");
		std::filesystem::create_directories(localePath);

		std::vector<UUID> stringTableIDs;
		EditorAssetDatabase::GetAllAssetsOfType(ResourceTypes::GetHash<StringTable>(), stringTableIDs);

		task.m_TotalSubTasks = stringTableIDs.size();

		for (const UUID stringTableID : stringTableIDs)
		{
			//EditableResource* pResource = EditorApplication::GetInstance()->GetResourceManager().GetEditableResource(stringTableID);
			//YAMLResource<StringTable>* pStringTableData = static_cast<YAMLResource<StringTable>*>(pResource);

			const std::string name = EditorAssetDatabase::GetAssetName(stringTableID);
			task.m_SubTaskName = name;

			Resource* pResource = pEngine->GetAssetManager().FindResource(stringTableID);
			if (!pResource) continue;
			StringTable* pStringTable = static_cast<StringTable*>(pResource);
			
			std::filesystem::path path = localePath;
			path.append(std::to_string(stringTableID)).replace_extension(".gcl");
			BinaryFileStream fileStream{ path };
			AssetArchive archive{ &fileStream, AssetArchiveFlags::WriteNew };
			archive.Serialize(pStringTable);

			++task.m_ProcessedSubTasks;
			task.m_SubTaskName = "";
		}
	}

	void LocalizeEditorExtension::Initialize()
	{
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		Reflect::SetReflectInstance(&pEngine->Reflection());
		pEngine->GetSceneManager()->ComponentTypesInstance();

		EditorApplication* pApp = EditorApplication::GetInstance();
		MainEditor& editor = pApp->GetMainEditor();

		Importer::Register(&Importer);
		Tumbnail::AddGenerator<StringTableTumbnailGenerator>();
		ObjectMenu::AddMenuItem("Create/String Table", OnCreateStringTable, ObjectMenuType::T_ContentBrowser | ObjectMenuType::T_Resource | ObjectMenuType::T_Folder);
		//MenuBar::AddMenuItem("Window/Localize/String Table Editor", [&editor]() { editor.GetWindow<StringTableEditor>(); }, NULL, Shortcut_Window_StringTableEditor);

		EditorPlayer::RegisterLoopHandler(this);

		//GatherPackageTasksEvents().AddListener([&](const EmptyEvent&) {
		//	PackageTask stringTablesTask;
		//	stringTablesTask.m_TotalSubTasks = 1;
		//	stringTablesTask.m_TaskID = "PackageStringTables";
		//	stringTablesTask.m_TaskName = "Packaging string tables";
		//	stringTablesTask.m_Callback = [this](Glory::Engine* pEngine, const std::filesystem::path& packageRoot, PackageTaskState& task) {
		//		PackageStringTables(pEngine, packageRoot, task);
		//		return true;
		//	};
		//	AddPackagingTaskAfter(std::move(stringTablesTask), "PackageAssets");

		//	/*PackageTask localeTask;
		//	localeTask.m_TotalSubTasks = 1;
		//	localeTask.m_TaskID = "PackageLocale";
		//	localeTask.m_TaskName = "Packaging locale data";
		//	localeTask.m_Callback = [this](Glory::Engine* pEngine, const std::filesystem::path& packageRoot, PackageTaskState& task) {
		//		
		//		return true;
		//	};
		//	AddPackagingTaskAfter(std::move(localeTask), "PackageStringTables");*/
		//});

		EntitySceneObjectEditor::AddComponentIcon<Localize>(ICON_FA_LANGUAGE);

		OBJECT_CREATE_MENU(Localize, Localize);
	}

	const char* LocalizeEditorExtension::ModuleName()
	{
		return "Localize";
	}

	void LocalizeEditorExtension::HandleBeforeStart(Module* pModule)
	{
		static_cast<LocalizeModule*>(pModule)->Clear();
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
