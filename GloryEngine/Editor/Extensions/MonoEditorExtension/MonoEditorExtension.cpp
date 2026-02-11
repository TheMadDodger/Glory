#include "MonoEditorExtension.h"
#include "MonoScriptThumbnail.h"
#include "MonoScriptImporter.h"
#include "MonoScriptComponentEditor.h"
#include "EditorAssetManager.h"
#include "ScriptTypeReferenceDrawer.h"

#include <Debug.h>
#include <Engine.h>
#include <AssetDatabase.h>
#include <MonoManager.h>
#include <AssemblyDomain.h>
#include <MonoScript.h>
#include <GloryMonoScipting.h>
#include <Package.h>
#include <Dispatcher.h>

#include <EditorAssetDatabase.h>
#include <EditorPreferencesWindow.h>
#include <EditorAssetCallbacks.h>
#include <ObjectMenuCallbacks.h>
#include <FileBrowser.h>
#include <AssetCompiler.h>
#include <ThumbnailManager.h>
#include <EntitySceneObjectEditor.h>
#include <ScriptingExtender.h>
#include <MainEditor.h>
#include <CreateEntityObjectsCallbacks.h>
#include <EditorApplication.h>
#include <MenuBar.h>
#include <CreateObjectAction.h>
#include <EditableEntity.h>
#include <EntityEditor.h>
#include <SystemTools.h>
#include <SceneManager.h>
#include <CoreLibManager.h>
#include <BinaryStream.h>

#include <fstream>
#include <string>
#include <tchar.h>
#include <locale>
#include <codecvt>
#include <windows.h>
#include <tinyfiledialogs.h>

#include <IconsFontAwesome6.h>

EXTENSION_CPP(MonoEditorExtension)

namespace Glory::Editor
{
	size_t MonoEditorExtension::m_CompilationCounter = 0;

	efsw::WatchID FileWatch = -1;

	std::atomic_bool ShouldReload = false;

	CREATE_OBJECT_CALLBACK_CPP(Scripted, MonoScriptComponent, ());

	GloryMonoScipting* MonoEditorExtension::m_pMonoScriptingModule = nullptr;

	MonoScriptImporter ScriptImporter;

	void MonoEditorExtension::HandleStop(Module* pModule)
	{
		MonoManager::Instance()->WaitForPendingFinalizers();
		MonoManager::Instance()->Reset(pModule->GetEngine());
	}

	void MonoEditorExtension::OnBeginPackage(const std::filesystem::path& path)
	{

	}

	void MonoEditorExtension::OnGenerateConfigExec(std::ofstream& stream)
	{
	}

	void MonoEditorExtension::OnEndPackage(const std::filesystem::path& path)
	{
#ifdef _DEBUG
		const bool release = false;
#else
		const bool release = true;
#endif
		/* Compile and copy assembly */
		ProjectSpace* pProject = ProjectSpace::GetOpenProject();
		CompileProject(pProject, release);

		const std::string mainAssembly = pProject->Name() + ".dll";
		std::filesystem::path assemblyPath = pProject->LibraryPath();
		assemblyPath.append("Assembly").append(mainAssembly);

		std::filesystem::copy(assemblyPath, path, std::filesystem::copy_options::overwrite_existing);

		/* Write Assemblies.dat */
		std::filesystem::path assembliesPath = path;
		assembliesPath.append("Data/Assemblies.dat");
		BinaryFileStream fileStream{ assembliesPath };
		BinaryStream& stream = fileStream;

		stream.Write(CoreVersion);
		std::vector<std::string> assemblies;
		assemblies.push_back(mainAssembly);
		stream.Write(assemblies);
	}

	void MonoEditorExtension::handleFileAction(efsw::WatchID watchid, const std::string& dir, const std::string& filename, efsw::Action action, std::string oldFilename)
	{
		std::filesystem::path filePath = dir;
		filePath.append(filename);

		switch (action)
		{
		case efsw::Actions::Add:
		case efsw::Actions::Modified:
			if (filePath.extension().compare(".dll") != 0) break;
			ShouldReload = true;
			break;
		}
	}

	MonoEditorExtension::MonoEditorExtension()
	{
	}

	MonoEditorExtension::~MonoEditorExtension()
	{
	}

	void MonoEditorExtension::OpenCSharpProject()
	{
		ProjectSpace* pProject = ProjectSpace::GetOpenProject();
		std::filesystem::path solutionPath = pProject->RootPath();
		solutionPath = solutionPath.append(pProject->Name() + ".csproj");

		GeneratePremakeFile(pProject);
		GenerateBatchFile(pProject);
		RunGenerateProjectFilesBatch(pProject);

		std::string windowName = pProject->Name() + " - Microsoft Visual Studio";
		HWND handle = FindWindowA(NULL, windowName.c_str());
		if (handle == NULL)
		{
			OpenFile(solutionPath);

			while (handle == NULL)
				handle = FindWindowA(NULL, windowName.c_str());
		}

		SetForegroundWindow(handle);
	}

	void MonoEditorExtension::OpenFile(const std::filesystem::path& path)
	{
		ShellExecute(0, L"open", path.wstring().c_str(), 0, 0, SW_SHOW);
	}

	size_t MonoEditorExtension::CompilationVersion()
	{
		return m_CompilationCounter;
	}

	void MonoEditorExtension::Initialize()
	{
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();

		m_pMonoScriptingModule = pEngine->GetOptionalModule<GloryMonoScipting>();
		Reflect::SetReflectInstance(&pEngine->Reflection());
		pEngine->GetSceneManager()->ComponentTypesInstance();

		EditorApplication* pEditorApp = EditorApplication::GetInstance();
		EditorSettings& settings = pEditorApp->GetMainEditor().Settings();
		std::filesystem::path visualStudioPath = settings["Mono/VisualStudioPath"].As<std::string>("");
		if (!FindMSBuildInVSPath(visualStudioPath, std::filesystem::path{}))
			FindVisualStudioPath();

		ProjectSpace::RegisterCallback(ProjectCallback::OnClose,
			[this](ProjectSpace* pProject) { OnProjectClose(pProject); });
		ProjectSpace::RegisterCallback(ProjectCallback::OnOpen,
			[this](ProjectSpace* pProject) { OnProjectOpen(pProject); });

		ObjectMenu::AddMenuItem("Create/Script/C#", MonoEditorExtension::OnCreateScript, ObjectMenuType::T_ContentBrowser | ObjectMenuType::T_Resource | ObjectMenuType::T_Folder);
		ObjectMenu::AddMenuItem("Open C# Project", MonoEditorExtension::OnOpenCSharpProject, ObjectMenuType::T_ContentBrowser | ObjectMenuType::T_Resource | ObjectMenuType::T_Folder);

		MenuBar::AddMenuItem("File/Compile C# Project", []() { CompileProject(ProjectSpace::GetOpenProject()); });

		ThumbnailManager::AddGenerator<MonoScriptThumbnail>();

		EditorAssetCallbacks::RegisterCallback(AssetCallbackType::CT_AssetUpdated, AssetCallback);
		EditorPreferencesWindow::AddPreferencesTab({ "Mono", [this]() { Preferences(); } });

		EditorPlayer::RegisterLoopHandler(this);

		Importer::Register(&ScriptImporter);

		Editor::RegisterEditor<MonoScriptComponentEditor>();
		PropertyDrawer::RegisterPropertyDrawer<PropertyDrawerTemplate<ScriptTypeReference>>();
		EntitySceneObjectEditor::AddComponentIcon<MonoScriptComponent>(ICON_FA_FILE_CODE);

		OBJECT_CREATE_MENU(Scripted, MonoScriptComponent);

		GatherPackageTasksEvents().AddListener([&](const EmptyEvent&) {
			PackageTask task;
			task.m_TotalSubTasks = 1;
			task.m_TaskID = "CompilScripting-Mono";
			task.m_TaskName = "Compiling mono scripting assembly";
			task.m_Callback = [this](Glory::Engine*, const std::filesystem::path& packageRoot, PackageTaskState& task) {
				OnEndPackage(packageRoot);
				++task.m_ProcessedSubTasks;
				return true;
			};
			AddPackagingTask(std::move(task), "CompileEXE");
		});
	}

	void MonoEditorExtension::Update()
	{
		if (!ShouldReload) return;
		ReloadAssembly(ProjectSpace::GetOpenProject());
		ShouldReload = false;
	}

	void MonoEditorExtension::FindVisualStudioPath()
	{
		EditorApplication* pEditorApp = EditorApplication::GetInstance();
		EditorSettings& settings = pEditorApp->GetMainEditor().Settings();

		std::filesystem::path msBuildPath;

		if (FindVisualStudio(msBuildPath))
		{
			settings["Mono/VisualStudioPath"].Set(msBuildPath.string());
			pEditorApp->GetEngine()->GetDebug().LogInfo("Found Visual Studio at " + settings["Mono/VisualStudioPath"].As<std::string>());
			return;
		}

		pEditorApp->GetEngine()->GetDebug().LogWarning("Could not find Visual Studio installation");
	}

	void MonoEditorExtension::OnProjectClose(ProjectSpace* pProject)
	{
		EditorApplication* pEditorApp = EditorApplication::GetInstance();
		if (FileWatch != -1)
			pEditorApp->FileWatch().removeWatch(FileWatch);
	}

	void MonoEditorExtension::OnProjectOpen(ProjectSpace* pProject)
	{
		GeneratePremakeFile(pProject);
		GenerateBatchFile(pProject);
		RunGenerateProjectFilesBatch(pProject);
		CompileProject(pProject);

		std::string name = pProject->Name() + ".dll";
		std::filesystem::path path = pProject->ProjectPath();
		path = path.parent_path().append("Library\\Assembly");
		/* TODO: Lib manager for user assemblies */

		EditorApplication* pEditorApp = EditorApplication::GetInstance();

		if (FileWatch != -1)
			pEditorApp->FileWatch().removeWatch(FileWatch);
		FileWatch = pEditorApp->FileWatch().addWatch(path.string(), this, false);

		m_pMonoScriptingModule->GetMonoManager()->AddLib(ScriptingLib(name, path.string(), true, nullptr));
	}

	void MonoEditorExtension::OnCreateScript(Object* pObject, const ObjectMenuType& menuType)
	{
		std::filesystem::path path = FileBrowserItem::GetCurrentPath();
		path = GetUnqiueFilePath(path.append("New CSharp Script.cs"));

		FileBrowser::BeginCreate(path.filename().replace_extension("").string(), "", [](std::filesystem::path& finalPath) {
			const std::string scriptName = finalPath.filename().replace_extension().string();
			finalPath.replace_extension("cs");
			if (std::filesystem::exists(finalPath)) return;

			std::string ns = m_pMonoScriptingModule->Settings().Value<std::string>("Default Namespace");
			if (ns == "$ProjectName")
				ns = ProjectSpace::GetOpenProject()->Name();

			const std::string extraTab = ns.empty() ? "" : "	";

			std::stringstream str;
			str << "using GloryEngine.Entities;" << std::endl << std::endl;
			if (!ns.empty())
			{
				str << "namespace " << ns << std::endl;
				str << "{" << std::endl;
			}
			str << extraTab << "public class " << scriptName << " : EntityBehaviour" << std::endl;
			str << extraTab << "{" << std::endl;
			str << extraTab << "	void Start()" << std::endl;
			str << extraTab << "	{" << std::endl;
			str << extraTab << "		// Start is called for the first frame regardless of active state" << std::endl;
			str << extraTab << "	}" << std::endl << std::endl;
			str << extraTab << "	void Stop()" << std::endl;
			str << extraTab << "	{" << std::endl;
			str << extraTab << "		// Stop is called when this component is destroyed, regardless of active state" << std::endl;
			str << extraTab << "	}" << std::endl << std::endl;
			str << extraTab << "	void OnEnable()" << std::endl;
			str << extraTab << "	{" << std::endl;
			str << extraTab << "		// OnEnable is called when the component switches from inactive to active" << std::endl;
			str << extraTab << "	}" << std::endl << std::endl;
			str << extraTab << "	void OnDisable()" << std::endl;
			str << extraTab << "	{" << std::endl;
			str << extraTab << "		// OnDisable is called when the component switches from active to inactive" << std::endl;
			str << extraTab << "	}" << std::endl << std::endl;
			str << extraTab << "	void OnValidate()" << std::endl;
			str << extraTab << "	{" << std::endl;
			str << extraTab << "		// OnValidate is called by the editor when a property on the component changes" << std::endl;
			str << extraTab << "		// It is also called once before start" << std::endl;
			str << extraTab << "	}" << std::endl << std::endl;
			str << extraTab << "	void Update()" << std::endl;
			str << extraTab << "	{" << std::endl;
			str << extraTab << "		// Update is called once every frame if the component is active" << std::endl;
			str << extraTab << "	}" << std::endl << std::endl;
			str << extraTab << "	void Draw()" << std::endl;
			str << extraTab << "	{" << std::endl;
			str << extraTab << "		// Draw is called once every frame during draw" << std::endl;
			str << extraTab << "	}" << std::endl;
			str << extraTab << "}" << std::endl;
			if (!ns.empty())
				str << "}" << std::endl;

			const std::string fileStr = str.str();
			std::vector<char> fileData;
			fileData.resize(fileStr.size());
			std::memcpy(fileData.data(), fileStr.data(), fileStr.size());

			MonoScript* pMonoScript = new MonoScript(std::move(fileData));
			EditorAssetDatabase::CreateAsset(pMonoScript, finalPath.string());
			FileBrowserItem::GetSelectedFolder()->Refresh();
			FileBrowserItem::GetSelectedFolder()->SortChildren();
		});
	}

	void MonoEditorExtension::OnOpenCSharpProject(Object* pObject, const ObjectMenuType& menuType)
	{
		OpenCSharpProject();
	}

	void MonoEditorExtension::CopyEngineAssemblies(ProjectSpace* pProject)
	{
		std::filesystem::path assembliesPath = pProject->LibraryPath();
		assembliesPath.append("Assembly");

		ScriptingExtender* pScriptingExtender = m_pMonoScriptingModule->GetScriptingExtender();
		for (size_t i = 0; i < pScriptingExtender->InternalLibCount(); ++i)
		{
			const ScriptingLib& lib = pScriptingExtender->GetInternalLib(i);
			std::filesystem::path path = lib.Location();
			path.append(lib.LibraryName());
			std::filesystem::path newPath = assembliesPath;
			newPath.append(lib.LibraryName());
			std::filesystem::copy_file(path, newPath, std::filesystem::copy_options::overwrite_existing);
		}
	}

	void MonoEditorExtension::GeneratePremakeFile(ProjectSpace* pProject)
	{
		ScriptingExtender* pScriptingExtender = m_pMonoScriptingModule->GetScriptingExtender();

		const std::string projectName = pProject->Name();
		// TODO: Make this setable in engine settings later
		const std::string dotNetFramework = "4.7.2";

		std::filesystem::path cachePath = pProject->CachePath();
		std::filesystem::path luaPath = cachePath;
		luaPath.append("Premake");
		if (!std::filesystem::exists(luaPath)) std::filesystem::create_directory(luaPath);
		luaPath.append("premake5.lua");
		std::ofstream luaStream(luaPath);
		std::filesystem::path editorPath = std::filesystem::current_path();
		std::filesystem::path premakePath = editorPath.append("premake").append("premake5.exe");
		editorPath = std::filesystem::current_path();
		luaStream << "workspace \"" << projectName << "\"" << std::endl;
		luaStream << "	platforms { \"x64\" }" << std::endl;
		luaStream << "	configurations { \"Debug\", \"Release\" }" << std::endl;
		luaStream << "project \"" << projectName << "\"" << std::endl;
		luaStream << "	kind \"SharedLib\"" << std::endl;
		luaStream << "	language \"C#\"" << std::endl;
		luaStream << "	namespace (\"" << projectName << "\")" << std::endl;
		luaStream << "	dotnetframework \"" << dotNetFramework << "\"" << std::endl;
		luaStream << "	targetdir \"" << "Library/Assembly" << "\"" << std::endl;
		luaStream << "	objdir \"" << "Library/OBJ" << "\"" << std::endl;
		luaStream << "	files \"" << "Assets/**.cs" << "\"" << std::endl;
		luaStream << "	defines \"" << "TRACE" << "\"" << std::endl;
		luaStream << "	libdirs " << std::endl;
		luaStream << "	{" << std::endl;
		size_t scriptingExtenderCount = pScriptingExtender->InternalLibCount();
		for (size_t i = 0; i < scriptingExtenderCount; i++)
		{
			const ScriptingLib& lib = pScriptingExtender->GetInternalLib(i);
			std::filesystem::path path = editorPath;
			path.append(lib.Location());
			luaStream << "		" << path;
			if (i != scriptingExtenderCount - 1) luaStream << ",";
			luaStream << std::endl;
		}
		luaStream << "	}" << std::endl;
		luaStream << "	links " << std::endl;
		luaStream << "	{" << std::endl;
		for (size_t i = 0; i < scriptingExtenderCount; i++)
		{
			const ScriptingLib& lib = pScriptingExtender->GetInternalLib(i);
			luaStream << "		\"" << lib.LibraryName() << "\"";
			if (i != scriptingExtenderCount - 1) luaStream << ",";
			luaStream << std::endl;
		}
		luaStream << "	}" << std::endl;
		luaStream << "	filter \"" << "platforms:x64" << "\"" << std::endl;
		luaStream << "		architecture \"" << "x64" << "\"" << std::endl;
		luaStream << "	filter \"" << "configurations:Debug" << "\"" << std::endl;
		luaStream << "		optimize \"" << "Off" << "\"" << std::endl;
		luaStream << "		defines \"" << "DEBUG" << "\"" << std::endl;
		luaStream << "		symbols \"" << "Default" << "\"" << std::endl;
		luaStream << "	filter \"" << "configurations:Release" << "\"" << std::endl;
		luaStream << "		optimize \"" << "On" << "\"" << std::endl;
		luaStream << "		defines \"" << "NDEBUG" << "\"" << std::endl;
		luaStream << "		symbols \"" << "Default" << "\"" << std::endl;
		luaStream.close();

		std::filesystem::path tempLuaPath = pProject->RootPath();
		tempLuaPath = tempLuaPath.append("premake5.lua");
		std::filesystem::copy_file(luaPath, tempLuaPath, std::filesystem::copy_options::overwrite_existing);
	}

	void MonoEditorExtension::GenerateBatchFile(ProjectSpace* pProject)
	{
		std::filesystem::path cachePath = pProject->CachePath();
		std::filesystem::path batchPath = cachePath.append("Premake").append("generateprojectfiles.bat");
		std::ofstream batchStream(batchPath);
		std::filesystem::path editorPath = std::filesystem::current_path();
		std::filesystem::path premakePath = editorPath.append("premake").append("premake5.exe");
		batchStream << "@echo off" << std::endl;
		batchStream << "pushd ..\\..\\" << std::endl;
		batchStream << "call " << premakePath << " vs2019" << std::endl;
		batchStream << "popd";
		batchStream.close();
	}

	void MonoEditorExtension::RunGenerateProjectFilesBatch(ProjectSpace* pProject)
	{
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));

		LPTSTR applicationName = _tcsdup(TEXT("c:\\Windows\\System32\\cmd.exe"));
		LPTSTR commandLine = _tcsdup(TEXT("/c generateprojectfiles.bat"));

		std::filesystem::path cachePath = pProject->CachePath();
		std::filesystem::path batchRootPath = cachePath.append("Premake");
		std::wstring pathWString = batchRootPath.wstring();
		LPCWSTR lpcWPath = pathWString.c_str();

		// Start the child process.
		if (!CreateProcess(applicationName,   // No module name (use command line)
			commandLine,        // Command line
			NULL,           // Process handle not inheritable
			NULL,           // Thread handle not inheritable
			FALSE,          // Set handle inheritance to FALSE
			0,              // No creation flags
			NULL,           // Use parent's environment block
			lpcWPath,       // Use current projects starting directory
			&si,            // Pointer to STARTUPINFO structure
			&pi)			// Pointer to PROCESS_INFORMATION structure
			)
		{
			printf("CreateProcess failed (%d).\n", GetLastError());
			return;
		}

		// Wait until child process exits.
		WaitForSingleObject(pi.hProcess, INFINITE);

		// Close process and thread handles.
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);

		std::filesystem::path tempLuaPath = pProject->RootPath();
		tempLuaPath = tempLuaPath.append("premake5.lua");
		std::filesystem::remove(tempLuaPath);
	}

	void MonoEditorExtension::CompileProject(ProjectSpace* pProject, bool release)
	{
		EditorApplication* pEditorApp = EditorApplication::GetInstance();
		pEditorApp->StopPlay();

		std::filesystem::path projectPath = pProject->RootPath();
		projectPath = projectPath.append(pProject->Name() + ".csproj");

		EditorSettings& settings = pEditorApp->GetMainEditor().Settings();

		std::filesystem::path msBuildPath = settings["Mono/VisualStudioPath"].As<std::string>("");
		if (!FindMSBuildInVSPath(msBuildPath, std::filesystem::path{}))
		{
			pEditorApp->GetEngine()->GetDebug().LogError("Could not compile C# project because a valid path to a Visual Studio installation is not specified!");
			return;
		}

		const std::string config = release ? "Release" : "Debug";

		std::string cmd = "cd \"" + msBuildPath.parent_path().string() + "\" && " + "msbuild /m /p:Configuration=" + config + " /p:Platform=x64 \"" + projectPath.string() + "\"";
		system(cmd.c_str());

		++m_CompilationCounter;
	}

	void MonoEditorExtension::ReloadAssembly(ProjectSpace* pProject)
	{
		m_pMonoScriptingModule->GetMonoManager()->Reload();

		/* Re-validate all script components */
		SceneManager* pScenes = m_pMonoScriptingModule->GetEngine()->GetSceneManager();
		for (size_t i = 0; i < pScenes->OpenScenesCount(); ++i)
		{
			pScenes->GetOpenScene(i)->GetRegistry().InvokeAll(MonoScriptComponent::GetTypeData()->TypeHash(), Utils::ECS::InvocationType::OnValidate);
		}
	}

	void MonoEditorExtension::AssetCallback(const AssetCallbackData& callback)
	{
		ResourceMeta meta;
		EditorAssetDatabase::GetAssetMetadata(callback.m_UUID, meta);
		uint32_t typeHash = meta.Hash();
		size_t scriptHash = ResourceTypes::GetHash<MonoScript>();
		ResourceTypes& types = EditorApplication::GetInstance()->GetEngine()->GetResourceTypes();
		ResourceType* pResourcerType = types.GetResourceType(typeHash);

		size_t subTypesCount = types.SubTypeCount(pResourcerType);
		for (size_t i = 0; i < subTypesCount; i++)
		{
			size_t subHash = types.GetSubTypeHash(pResourcerType, i);
			if (scriptHash != subHash) continue;
			EditorApplication::GetInstance()->GetAssetManager().UnloadAsset(callback.m_UUID);
			if (!AssetCompiler::IsCompilingAsset(callback.m_UUID))
			{
				AssetCompiler::CompileAssetsImmediately({ callback.m_UUID });
			}
			CompileProject(ProjectSpace::GetOpenProject());
			return;
		}
	}

	void MonoEditorExtension::Preferences()
	{
		ImGui::TextUnformatted("Mono");
		ImGui::Separator();
		ImGui::Spacing();

		EditorApplication* pEditorApp = EditorApplication::GetInstance();
		EditorSettings& settings = pEditorApp->GetMainEditor().Settings();
		const std::filesystem::path visualStudioPath = settings["Mono/VisualStudioPath"].As<std::string>("");

		ImGui::Text("Visual Studio: %s", visualStudioPath.string().c_str());
		ImGui::SameLine();
		const float availableWidth = ImGui::GetContentRegionAvail().x;
		const float cursorX = ImGui::GetCursorPosX();
		ImGui::SetCursorPosX(cursorX + availableWidth - 100);
		if (ImGui::Button("Browse", { 100.0f, 0.0f }))
		{
			const char* filters[1] = { "*.gproj" };
			const char* path = tinyfd_selectFolderDialog("Select Visual Studio Installation", visualStudioPath.string().data());

			if (!path) return;

			if (FindMSBuildInVSPath(std::filesystem::path(path), std::filesystem::path{}))
			{
				settings["Mono/VisualStudioPath"].Set(path);
				return;
			}
			pEditorApp->GetEngine()->GetDebug().LogWarning("Invalid Visual Studio Path");
		}
	}
}
