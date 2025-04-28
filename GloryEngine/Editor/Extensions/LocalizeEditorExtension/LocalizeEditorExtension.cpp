#include "LocalizeEditorExtension.h"
#include "StringTableImporter.h"
#include "StringTableTumbnailGenerator.h"
#include "LanguageSettings.h"
#include "StringTableEditor.h"

#include <Localize.h>
#include <StringsOverrideTable.h>
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
	CREATE_OBJECT_CALLBACK_CPP(StringTableLoader, StringTableLoader, ());

	static constexpr char* Shortcut_Window_StringTableEditor = "Open String Table Editor";

	StringTableImporter Importer;
	LanguageSettings LangSettings;

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

			pStringTable->AddString("test", "Hello World!");
			pStringTable->AddString("testin.test1", "Hello World! 1");
			pStringTable->AddString("testin.test2", "Hello World! 2");
			pStringTable->AddString("testin.test3.test", "Hello World! deep");
			pStringTable->AddString("testin.test3.testtest", "Hello World! deep 2");
			pStringTable->AddString("testin.testin.testin.testin.testin.testin", "Hello World! super deep");

			EditorAssetDatabase::CreateAsset(pStringTable, finalPath.string());
			FileBrowserItem::GetSelectedFolder()->Refresh();
			FileBrowserItem::GetSelectedFolder()->SortChildren();
		});
	}

	void OnCreateStringOverrideTable(Object* pBaseTable, const ObjectMenuType&)
	{
		std::filesystem::path path = FileBrowserItem::GetCurrentPath();
		path = GetUnqiueFilePath(path.append("New String Override Table.gotable"));

		FileBrowser::BeginCreate(path.filename().replace_extension("").string(), "", [pBaseTable](std::filesystem::path& finalPath) {
			finalPath.replace_extension("gotable");
			if (std::filesystem::exists(finalPath)) return;

			StringTable* pStringTable = nullptr;
			if (pBaseTable)
			{
				std::type_index type = typeid(Object);
				pBaseTable->GetType(0, type);
				if (type == typeid(StringTable)) pStringTable = static_cast<StringTable*>(pBaseTable);
			}

			const UUID baseTableID = pStringTable ? pStringTable->GetUUID() : 0;
			std::string language = "English";

			StringsOverrideTable* pStringOverrideTable = new StringsOverrideTable(baseTableID, std::move(language));
			EditorAssetDatabase::CreateAsset(pStringOverrideTable, finalPath.string());
			FileBrowserItem::GetSelectedFolder()->Refresh();
			FileBrowserItem::GetSelectedFolder()->SortChildren();
		});
	}

	std::vector<LocaleData> LocaleDatas;
	void PackageStringOverrideTables(Glory::Engine* pEngine, const std::filesystem::path& packageRoot, PackageTaskState& task)
	{
		LocaleDatas.clear();

		std::filesystem::path localePath = packageRoot;
		localePath.append("Data/Locale");
		std::filesystem::create_directories(localePath);

		std::vector<UUID> stringTableIDs;
		EditorAssetDatabase::GetAllAssetsOfType(ResourceTypes::GetHash<StringsOverrideTable>(), stringTableIDs);

		task.m_TotalSubTasks = stringTableIDs.size();

		for (const UUID stringTableID : stringTableIDs)
		{
			//EditableResource* pResource = EditorApplication::GetInstance()->GetResourceManager().GetEditableResource(stringTableID);
			//YAMLResource<StringTable>* pStringTableData = static_cast<YAMLResource<StringTable>*>(pResource);

			const std::string name = EditorAssetDatabase::GetAssetName(stringTableID);
			task.m_SubTaskName = name;

			Resource* pResource = pEngine->GetAssetManager().FindResource(stringTableID);
			if (!pResource)
			{
				++task.m_ProcessedSubTasks;
				continue;
			}
			StringsOverrideTable* pStringTable = static_cast<StringsOverrideTable*>(pResource);
			if (pStringTable->BaseTableID() == 0 || pStringTable->Language().empty() || !IsAssetPackaged(pStringTable->BaseTableID()))
			{
				++task.m_ProcessedSubTasks;
				continue;
			}

			std::filesystem::path path = localePath;
			path.append(std::to_string(stringTableID)).replace_extension(".gcl");
			BinaryFileStream fileStream{ path };
			AssetArchive archive{ &fileStream, AssetArchiveFlags::WriteNew };
			archive.Serialize(pStringTable);

			LocaleData localeData;
			localeData.m_BaseTableID = pStringTable->BaseTableID();
			localeData.m_OverrideTableID = pStringTable->GetUUID();
			localeData.m_Language = pStringTable->Language();
			LocaleDatas.push_back(std::move(localeData));

			++task.m_ProcessedSubTasks;
			task.m_SubTaskName = "";
		}
	}

	void PackageLocaleData(Glory::Engine* pEngine, const std::filesystem::path& packageRoot, PackageTaskState& task)
	{
		task.m_SubTaskName = "Locale.dat";

		std::filesystem::path localeDataPath = packageRoot;
		localeDataPath.append("Data/Locale.dat");
		const std::filesystem::path localePath = "./Data/Locale";

		BinaryFileStream file{ localeDataPath };
		BinaryStream* stream = &file;
		stream->Write(CoreVersion);
		stream->Write(LocaleDatas.size());
		for (const LocaleData& localeData : LocaleDatas)
		{
			stream->Write(localeData.m_BaseTableID).Write(localeData.m_OverrideTableID).Write(localeData.m_Language);
		}
	}

	void LocalizeEditorExtension::Initialize()
	{
		ProjectSettings::Add(&LangSettings);

		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		Reflect::SetReflectInstance(&pEngine->Reflection());
		pEngine->GetSceneManager()->ComponentTypesInstance();

		EditorApplication* pApp = EditorApplication::GetInstance();
		MainEditor& editor = pApp->GetMainEditor();

		Importer::Register(&Importer);
		Tumbnail::AddGenerator<StringTableTumbnailGenerator>();
		Tumbnail::AddGenerator<StringsOverrideTableTumbnailGenerator>();
		ObjectMenu::AddMenuItem("Create/Localize/String Table", OnCreateStringTable, ObjectMenuType::T_ContentBrowser | ObjectMenuType::T_Resource | ObjectMenuType::T_Folder);
		ObjectMenu::AddMenuItem("Create/Localize/String Override Table", OnCreateStringOverrideTable, ObjectMenuType::T_ContentBrowser | ObjectMenuType::T_Resource | ObjectMenuType::T_Folder);
		MenuBar::AddMenuItem("Window/Localize/String Table Editor", [&editor]() { editor.GetWindow<StringTableEditor>(); }, NULL, Shortcut_Window_StringTableEditor);

		EditorPlayer::RegisterLoopHandler(this);

		GatherPackageTasksEvents().AddListener([&](const EmptyEvent&) {
			PackageTask stringTablesTask;
			stringTablesTask.m_TotalSubTasks = 1;
			stringTablesTask.m_TaskID = "PackageStringOverrideTables";
			stringTablesTask.m_TaskName = "Packaging string override tables";
			stringTablesTask.m_Callback = [this](Glory::Engine* pEngine, const std::filesystem::path& packageRoot, PackageTaskState& task) {
				PackageStringOverrideTables(pEngine, packageRoot, task);
				return true;
			};
			AddPackagingTaskAfter(std::move(stringTablesTask), "PackageAssets");

			PackageTask localeTask;
			localeTask.m_TotalSubTasks = 1;
			localeTask.m_TaskID = "PackageLocale";
			localeTask.m_TaskName = "Packaging locale data";
			localeTask.m_Callback = [this](Glory::Engine* pEngine, const std::filesystem::path& packageRoot, PackageTaskState& task) {
				PackageLocaleData(pEngine, packageRoot, task);
				return true;
			};
			AddPackagingTaskAfter(std::move(localeTask), "PackageStringOverrideTables");
		});

		EntitySceneObjectEditor::AddComponentIcon<StringTableLoader>(ICON_FA_LANGUAGE);
		EntitySceneObjectEditor::AddComponentIcon<Localize>(ICON_FA_LANGUAGE);

		OBJECT_CREATE_MENU(StringTableLoader, StringTableLoader);
	}

	const char* LocalizeEditorExtension::ModuleName()
	{
		return "Localize";
	}

	void LocalizeEditorExtension::HandleBeforeStart(Module* pModule)
	{
		LocalizeModule* pLocalize = static_cast<LocalizeModule*>(pModule);
		pLocalize->Clear();

		std::vector<LocaleData> localeDatas;

		std::vector<UUID> stringOverrideTableIDs;
		EditorAssetDatabase::GetAllAssetsOfType(ResourceTypes::GetHash<StringsOverrideTable>(), stringOverrideTableIDs);

		for (size_t i = 0; i < stringOverrideTableIDs.size(); ++i)
		{
			Resource* pResource = pLocalize->GetEngine()->GetAssetManager().FindResource(stringOverrideTableIDs[i]);
			if (!pResource)
				continue;
			StringsOverrideTable* pStringTable = static_cast<StringsOverrideTable*>(pResource);
			if (pStringTable->BaseTableID() == 0 || pStringTable->Language().empty())
				continue;

			LocaleData localeData;
			localeData.m_BaseTableID = pStringTable->BaseTableID();
			localeData.m_Language = pStringTable->Language();
			localeData.m_OverrideTableID = pStringTable->GetUUID();
			localeDatas.emplace_back(std::move(localeData));
		}
		pLocalize->SetLocaleDatas(std::move(localeDatas));
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
