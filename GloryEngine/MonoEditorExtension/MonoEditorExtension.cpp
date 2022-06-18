#include "MonoEditorExtension.h"
#include "MonoScriptTumbnail.h"
#include <Game.h>
#include <Engine.h>
#include <ObjectMenuCallbacks.h>
#include <ContentBrowser.h>
#include <fstream>
#include <AssetDatabase.h>
#include <MonoScript.h>
#include <windows.h>
#include <Tumbnail.h>
#include <MonoManager.h>
#include <tchar.h>

#include <locale>
#include <codecvt>
#include <string>
#include <MonoLibManager.h>

namespace Glory::Editor
{
	ticpp::Document MonoEditorExtension::m_SolutionDocument;

	MonoEditorExtension::MonoEditorExtension()
	{
	}

	MonoEditorExtension::~MonoEditorExtension()
	{
	}

	void MonoEditorExtension::OpenCSharpProject()
	{
		ProjectSpace* pProject = ProjectSpace::GetOpenProject();
		std::filesystem::path projectPath = pProject->RootPath();
		projectPath = projectPath.append(pProject->Name() + ".csproj");

		GeneratePremakeFile(pProject);
		GenerateBatchFile(pProject);
		RunGenerateProjectFilesBatch(pProject);

		std::string windowName = pProject->Name() + " - Microsoft Visual Studio";
		HWND handle = FindWindowA(NULL, windowName.c_str());
		if (handle == NULL)
		{
			std::string cmd = "\"C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/Common7/IDE/devenv\" " + projectPath.string();
			system(cmd.c_str());

			while (handle == NULL)
				handle = FindWindowA(NULL, windowName.c_str());
		}

		SetForegroundWindow(handle);
	}

	void MonoEditorExtension::RegisterEditors()
	{
		m_pScriptingModule = Game::GetGame().GetEngine()->GetScriptingModule<GloryMonoScipting>();

		ProjectSpace::RegisterCallback(ProjectCallback::OnClose, MonoEditorExtension::OnProjectClose);
		ProjectSpace::RegisterCallback(ProjectCallback::OnOpen, MonoEditorExtension::OnProjectOpen);

		ObjectMenu::AddMenuItem("Create/Script/C#", MonoEditorExtension::OnCreateScript, ObjectMenuType::T_ContentBrowser | ObjectMenuType::T_Resource | ObjectMenuType::T_Folder);
		ObjectMenu::AddMenuItem("Open C# Project", MonoEditorExtension::OnOpenCSharpProject, ObjectMenuType::T_ContentBrowser | ObjectMenuType::T_Resource | ObjectMenuType::T_Folder);

		Tumbnail::AddGenerator<MonoScriptTumbnail>();
	}

	void MonoEditorExtension::OnProjectClose(ProjectSpace* pProject)
	{
	}

	void MonoEditorExtension::OnProjectOpen(ProjectSpace* pProject)
	{
		std::string name = pProject->Name() + ".dll";
		MonoManager::LoadLib(ScriptingLib("C#", name));
	}

	void MonoEditorExtension::OnCreateScript(Object* pObject, const ObjectMenuType& menuType)
	{
		std::filesystem::path path = ContentBrowserItem::GetCurrentPath();
		path = GetUnqiueFilePath(path.append("New CSharp Script.cs"));

		//std::ofstream outStream(path);
		//outStream.close();
		MonoScript* pMonoScript = new MonoScript();
		AssetDatabase::CreateAsset(pMonoScript, path.string());
		AssetDatabase::Save();

		ContentBrowserItem::GetSelectedFolder()->Refresh();
		ContentBrowserItem::GetSelectedFolder()->SortChildren();
		ContentBrowser::BeginRename(path.filename().string(), false);
	}

	void MonoEditorExtension::OnOpenCSharpProject(Object* pObject, const ObjectMenuType& menuType)
	{
		OpenCSharpProject();
	}

	void MonoEditorExtension::GeneratePremakeFile(ProjectSpace* pProject)
	{
		std::string projectName = pProject->Name();
		// TODO: Make this setable in engine settings later
		std::string dotNetFramework = "4.7.1";

		std::filesystem::path cachePath = pProject->CachePath();
		std::filesystem::path luaPath = cachePath.append("Premake").append("premake5.lua");
		std::ofstream luaStream(luaPath);
		std::filesystem::path editorPath = std::filesystem::current_path();
		std::filesystem::path premakePath = editorPath.append("premake").append("premake5.exe");
		editorPath = std::filesystem::current_path();
		luaStream << "workspace \"" << projectName << "\"" << std::endl;
		luaStream << "	platforms { \"Win32\", \"x64\" }" << std::endl;
		luaStream << "	configurations { \"Debug\", \"Release\" }" << std::endl;
		luaStream << "	flags { \"MultiProcessorCompile\" }" << std::endl;
		luaStream << "project \"Sponza\"" << std::endl;
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
		luaStream << "		" << editorPath << std::endl;
		luaStream << "	}" << std::endl;
		luaStream << "	links " << std::endl;
		luaStream << "	{" << std::endl;
		MonoLibManager::ForEachAssembly([&](AssemblyBinding* pAssembly)
		{
			luaStream << "		\"" << pAssembly->Name() << "\"," << std::endl;
		});
		luaStream << "		\"END\"" << std::endl;
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
		std::filesystem::copy(luaPath, tempLuaPath, std::filesystem::copy_options::overwrite_existing);
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
}

//<ItemGroup>
//<Compile Include = "Assets\Scripts\rtlkhyjtlhkrthikhbk.cs" />
//<Compile Include = "Assets\Scripts\TestScript.cs" />
//</ItemGroup>

//<ItemGroup>
//	<Reference Include = "AppLogger">
//		<HintPath>..\..\..\Glorious\TEST\CSharpTest\AppLogger\bin\Debug\netcoreapp3.1\AppLogger.dll< / HintPath>
//	< / Reference>
//< / ItemGroup>