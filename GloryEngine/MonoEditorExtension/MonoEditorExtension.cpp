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

		ReloadCSharpDocument(projectPath.string());
		UpdateCSharpProjectFile();

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

	void MonoEditorExtension::UpdateCSharpProjectFile()
	{
		size_t hash = ResourceType::GetHash<MonoScript>();
		std::vector<UUID> scriptFilePaths;
		AssetDatabase::GetAllAssetsOfType(hash, scriptFilePaths);

		ticpp::Element* pRootNode = m_SolutionDocument.FirstChildElement();
		ticpp::Element* pFirstSibling = pRootNode->FirstChildElement();
		ticpp::Element* pSibling = pFirstSibling->NextSiblingElement("ItemGroup", false);
		pSibling->Clear();

		for (size_t i = 0; i < scriptFilePaths.size(); i++)
		{
			ticpp::Element* pCompileElement = new ticpp::Element("Compile");
			pSibling->LinkEndChild(pCompileElement);

			UUID uuid = scriptFilePaths[i];
			AssetLocation location;
			if (!AssetDatabase::GetAssetLocation(uuid, location)) continue;

			std::filesystem::path path = std::filesystem::path("Assets").append(location.m_Path);
			pCompileElement->SetAttribute("Include", path.string());
		}

		// TODO: Relink dlls

		m_SolutionDocument.SaveFile();
	}

	void MonoEditorExtension::ReloadCSharpDocument(const std::string& projectPath)
	{
		if (!std::filesystem::exists(projectPath)) CreateCSharpDocument(projectPath);
		m_SolutionDocument = ticpp::Document(projectPath);
		m_SolutionDocument.LoadFile();
	}
	void MonoEditorExtension::CreateCSharpDocument(const std::string& projectPath)
	{
		std::filesystem::copy_file("./EditorAssets/Mono/CSProjTemplate.txt", projectPath);

		//ticpp::Element* root = new ticpp::Element("Project");
		//m_SolutionDocument.LinkEndChild(root);
		//root->SetAttribute("Sdk", "Microsoft.NET.Sdk");
		//
		//ticpp::Element* propertyGroup = new ticpp::Element("PropertyGroup");
		//root->LinkEndChild(propertyGroup);
		//
		//ticpp::Element* outputType = new ticpp::Element("OutputType");
		//propertyGroup->LinkEndChild(outputType);
		//
		//ticpp::Text* text = new ticpp::Text("Library");
		//outputType->LinkEndChild(text);
		//
		//ticpp::Element* appDesignerFolder = new ticpp::Element("AppDesignerFolder");
		//propertyGroup->LinkEndChild(appDesignerFolder);
		//
		//text = new ticpp::Text("Properties");
		//appDesignerFolder->LinkEndChild(text);
		//
		//ticpp::Element* element = new ticpp::Element("RootNamespace");
		//propertyGroup->LinkEndChild(element);
		//
		//text = new ticpp::Text(pProject->Name());
		//element->LinkEndChild(text);
		//
		//element = new ticpp::Element("TargetFramework");
		//propertyGroup->LinkEndChild(element);
		//
		//text = new ticpp::Text("netstandard2.1");
		//element->LinkEndChild(text);
		//
		//element = new ticpp::Element("EnableDefaultCompileItems");
		//propertyGroup->LinkEndChild(element);
		//
		//text = new ticpp::Text("false");
		//element->LinkEndChild(text);
		//
		//ticpp::Element* filesItemGroup = new ticpp::Element("ItemGroup");
		//root->LinkEndChild(filesItemGroup);
		//
		//
		//ticpp::Element* itemGroup = new ticpp::Element("ItemGroup");
		//root->LinkEndChild(itemGroup);
		//
		//ticpp::Element* reference = new ticpp::Element("Reference");
		//itemGroup->LinkEndChild(reference);
		//reference->SetAttribute("Include", "AppLogger");
		//
		//element = new ticpp::Element("HintPath");
		//reference->LinkEndChild(element);
		//
		//// TODO: scan for csharp dlls from other modules
		//text = new ticpp::Text("..\\..\\..\\Glorious\\TEST\\CSharpTest\\AppLogger\\bin\\Debug\\netcoreapp3.1\\AppLogger.dll");
		//element->LinkEndChild(text);
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