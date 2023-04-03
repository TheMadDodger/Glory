#include "MonoEditorExtension.h"
#include "MonoScriptTumbnail.h"
#include "FileDialog.h"

#include <EditorAssetDatabase.h>
#include <EditorPreferencesWindow.h>
#include <EditorAssetCallbacks.h>
#include <Game.h>
#include <Engine.h>
#include <ObjectMenuCallbacks.h>
#include <FileBrowser.h>
#include <fstream>
#include <AssetDatabase.h>
#include <MonoScript.h>
#include <Tumbnail.h>
#include <MonoManager.h>
#include <AssetManager.h>

#include <string>
#include <MonoLibManager.h>
#include <EditorApplication.h>

#include <tchar.h>
#include <locale>
#include <codecvt>
#include <windows.h>
#include <MenuBar.h>

Glory::Editor::BaseEditorExtension* LoadExtension()
{
	return new Glory::Editor::MonoEditorExtension();
}

void SetContext(Glory::GloryContext* pContext, ImGuiContext* pImGUIContext)
{
	Glory::GloryContext::SetContext(pContext);
	ImGui::SetCurrentContext(pImGUIContext);
}

namespace Glory::Editor
{
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

	void MonoEditorExtension::RegisterEditors()
	{
		EditorApplication* pEditorApp = EditorApplication::GetInstance();
		EditorSettings& settings = pEditorApp->GetMainEditor()->Settings();
		std::filesystem::path visualStudioPath = settings["Mono/VisualStudioPath"].As<std::string>("");
		if (!FindMSBuild(visualStudioPath))
			FindVisualStudioPath();

		ProjectSpace::RegisterCallback(ProjectCallback::OnClose, MonoEditorExtension::OnProjectClose);
		ProjectSpace::RegisterCallback(ProjectCallback::OnOpen, MonoEditorExtension::OnProjectOpen);

		ObjectMenu::AddMenuItem("Create/Script/C#", MonoEditorExtension::OnCreateScript, ObjectMenuType::T_ContentBrowser | ObjectMenuType::T_Resource | ObjectMenuType::T_Folder);
		ObjectMenu::AddMenuItem("Open C# Project", MonoEditorExtension::OnOpenCSharpProject, ObjectMenuType::T_ContentBrowser | ObjectMenuType::T_Resource | ObjectMenuType::T_Folder);

		MenuBar::AddMenuItem("File/Compile C# Project", CompileProject);

		Tumbnail::AddGenerator<MonoScriptTumbnail>();

		EditorAssetCallbacks::RegisterCallback(AssetCallbackType::CT_AssetUpdated, AssetCallback);
		EditorPreferencesWindow::AddPreferencesTab({ "Mono", [this]() { Preferences(); } });
	}

	void MonoEditorExtension::FindVisualStudioPath()
	{
		const std::filesystem::path x64Path = "C:/Program Files/Microsoft Visual Studio";
		const std::filesystem::path x86Path = "C:/Program Files (x86)/Microsoft Visual Studio";

		EditorApplication* pEditorApp = EditorApplication::GetInstance();
		EditorSettings& settings = pEditorApp->GetMainEditor()->Settings();

		if (FindVisualStudioPath(x64Path))
		{
			Debug::LogInfo("Found Visual Studio at " + settings["Mono/VisualStudioPath"].As<std::string>());
			return;
		}

		if (FindVisualStudioPath(x86Path))
		{
			Debug::LogInfo("Found Visual Studio at " + settings["Mono/VisualStudioPath"].As<std::string>());
			return;
		}

		Debug::LogWarning("Could not find Visual Studio installation");
	}

	bool MonoEditorExtension::FindVisualStudioPath(const std::filesystem::path& path)
	{
		EditorApplication* pEditorApp = EditorApplication::GetInstance();
		EditorSettings& settings = pEditorApp->GetMainEditor()->Settings();

		if (!std::filesystem::exists(path)) return false;
		for (const std::filesystem::directory_entry entry : std::filesystem::directory_iterator(path))
		{
			if (!entry.is_directory()) continue;
			const std::filesystem::path versionPath = entry.path();
			std::filesystem::path editionPath = versionPath;
			editionPath.append("Enterprise");
			if (FindMSBuild(std::filesystem::path(editionPath)))
			{
				settings["Mono/VisualStudioPath"].Set(editionPath.string());
				return true;
			}

			editionPath = versionPath;
			editionPath.append("Professional");
			if (FindMSBuild(std::filesystem::path(editionPath)))
			{
				settings["Mono/VisualStudioPath"].Set(editionPath.string());
				return true;
			}

			editionPath = versionPath;
			editionPath.append("Community");
			if (FindMSBuild(std::filesystem::path(editionPath)))
			{
				settings["Mono/VisualStudioPath"].Set(editionPath.string());
				return true;
			}
		}

		return false;
	}

	bool MonoEditorExtension::FindMSBuild(std::filesystem::path& path)
	{
		path.append("MSBuild/Current/Bin/MSBuild.exe");
		return std::filesystem::exists(path);
	}

	void MonoEditorExtension::OnProjectClose(ProjectSpace* pProject)
	{

	}

	void MonoEditorExtension::OnProjectOpen(ProjectSpace* pProject)
	{
		GeneratePremakeFile(pProject);
		GenerateBatchFile(pProject);
		RunGenerateProjectFilesBatch(pProject);

		std::string name = pProject->Name() + ".dll";
		std::filesystem::path path = pProject->ProjectPath();
		path = path.parent_path().append("Library/Assembly");
		/* TODO: Lib manager for uuser assemblies */
		MonoManager::LoadLib(ScriptingLib("csharp", name, path.string(), true, nullptr, true));
	}

	void MonoEditorExtension::OnCreateScript(Object* pObject, const ObjectMenuType& menuType)
	{
		std::filesystem::path path = FileBrowserItem::GetCurrentPath();
		path = GetUnqiueFilePath(path.append("New CSharp Script.cs"));

		MonoScript* pMonoScript = new MonoScript();
		EditorAssetDatabase::CreateAsset(pMonoScript, path.string());

		FileBrowserItem::GetSelectedFolder()->Refresh();
		FileBrowserItem::GetSelectedFolder()->SortChildren();
		FileBrowser::BeginRename(path.filename().string(), false);
	}

	void MonoEditorExtension::OnOpenCSharpProject(Object* pObject, const ObjectMenuType& menuType)
	{
		OpenCSharpProject();
	}

	void MonoEditorExtension::CopyEngineAssemblies(ProjectSpace* pProject)
	{
		std::filesystem::path assembliesPath = pProject->LibraryPath();
		assembliesPath.append("Assembly");

		ScriptingExtender* pScriptingExtender = Game::GetGame().GetEngine()->GetScriptingExtender();
		for (size_t i = 0; i < pScriptingExtender->InternalLibCount(); i++)
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
		ScriptingExtender* pScriptingExtender = Game::GetGame().GetEngine()->GetScriptingExtender();

		std::string projectName = pProject->Name();
		// TODO: Make this setable in engine settings later
		std::string dotNetFramework = "4.7.1";

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
		luaStream << "	platforms { \"Win32\", \"x64\" }" << std::endl;
		luaStream << "	configurations { \"Debug\", \"Release\" }" << std::endl;
		luaStream << "	flags { \"MultiProcessorCompile\" }" << std::endl;
		luaStream << "project \"" << projectName << "\"" << std::endl;
		luaStream << "	kind \"SharedLib\"" << std::endl;
		luaStream << "	language \"C#\"" << std::endl;
		luaStream << "	staticruntime \"Off\"" << std::endl;
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
		luaStream << "	filter \"" << "platforms:Win32" << "\"" << std::endl;
		luaStream << "		architecture \"" << "x86" << "\"" << std::endl;
		luaStream << "		defines \"" << "WIN32" << "\"" << std::endl;
		luaStream << "	filter \"" << "platforms:x64" << "\"" << std::endl;
		luaStream << "		architecture \"" << "x64" << "\"" << std::endl;
		luaStream << "	filter \"" << "configurations:Debug" << "\"" << std::endl;
		luaStream << "		runtime \"" << "Debug" << "\"" << std::endl;
		luaStream << "		defines \"" << "DEBUG" << "\"" << std::endl;
		luaStream << "		symbols \"" << "On" << "\"" << std::endl;
		luaStream << "	filter \"" << "configurations:Release" << "\"" << std::endl;
		luaStream << "		runtime \"" << "Release" << "\"" << std::endl;
		luaStream << "		defines \"" << "NDEBUG" << "\"" << std::endl;
		luaStream << "		optimize \"" << "On" << "\"" << std::endl;
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

	void MonoEditorExtension::CompileProject()
	{
		ProjectSpace* pProject = ProjectSpace::GetOpenProject();
		std::filesystem::path projectPath = pProject->RootPath();
		projectPath = projectPath.append(pProject->Name() + ".csproj");

		EditorApplication* pEditorApp = EditorApplication::GetInstance();
		EditorSettings& settings = pEditorApp->GetMainEditor()->Settings();

		std::filesystem::path msBuildPath = settings["Mono/VisualStudioPath"].As<std::string>();
		if (!FindMSBuild(msBuildPath))
		{
			Debug::LogError("Could not compile C# project because a valid path to a Visual Studio installation is not specified!");
			return;
		}

		projectPath.parent_path().string();

		std::string cmd = "cd \"" + msBuildPath.parent_path().string() + "\" && " + "msbuild /m /p:Configuration=Debug /p:Platform=x64 \"" + projectPath.string() + "\"";
		system(cmd.c_str());

		ReloadAssembly();
	}

	void MonoEditorExtension::ReloadAssembly()
	{
		MonoManager::Reload();
	}

	void MonoEditorExtension::AssetCallback(UUID uuid, const ResourceMeta& meta, Resource*)
	{
		uint32_t typeHash = meta.Hash();
		size_t scriptHash = ResourceType::GetHash<Script>();
		ResourceType* pResourcerType = ResourceType::GetResourceType(typeHash);

		size_t subTypesCount = ResourceType::SubTypeCount(pResourcerType);
		for (size_t i = 0; i < subTypesCount; i++)
		{
			size_t subHash = ResourceType::GetSubTypeHash(pResourcerType, i);
			if (scriptHash != subHash) continue;
			AssetManager::ReloadAsset(uuid);
			CompileProject();
			return;
		}
	}

	void MonoEditorExtension::Preferences()
	{
		ImGui::TextUnformatted("Mono");
		ImGui::Separator();
		ImGui::Spacing();

		EditorApplication* pEditorApp = EditorApplication::GetInstance();
		EditorSettings& settings = pEditorApp->GetMainEditor()->Settings();
		const std::filesystem::path visualStudioPath = settings["Mono/VisualStudioPath"].As<std::string>();

		ImGui::Text("Visual Studio: %s", visualStudioPath.string().c_str());
		ImGui::SameLine();
		const float availableWidth = ImGui::GetContentRegionAvail().x;
		const float cursorX = ImGui::GetCursorPosX();
		ImGui::SetCursorPosX(cursorX + availableWidth - 100);
		if (ImGui::Button("Browse", { 100.0f, 0.0f }))
		{
			FileDialog::Open("visualstudiopath", "Visual Studio Installation", "", false, visualStudioPath.string().c_str(), [&](const std::string& path) {
				if (FindMSBuild(std::filesystem::path(path)))
				{
					settings["Mono/VisualStudioPath"].Set(path);
					return;
				}
				Debug::LogWarning("Invalid Visual Studio Path");
			});
		}
	}
}
