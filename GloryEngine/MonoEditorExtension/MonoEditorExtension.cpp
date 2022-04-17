#include "MonoEditorExtension.h"
#include <Game.h>
#include <Engine.h>

namespace Glory::Editor
{
	ticpp::Document MonoEditorExtension::m_SolutionDocument;

	MonoEditorExtension::MonoEditorExtension()
	{
	}

	MonoEditorExtension::~MonoEditorExtension()
	{
	}

	void MonoEditorExtension::RegisterEditors()
	{
		m_pScriptingModule = Game::GetGame().GetEngine()->GetScriptingModule<GloryMonoScipting>();

		ProjectSpace::RegisterCallback(ProjectCallback::OnClose, MonoEditorExtension::OnProjectClose);
		ProjectSpace::RegisterCallback(ProjectCallback::OnOpen, MonoEditorExtension::OnProjectOpen);
	}

	void MonoEditorExtension::OnProjectClose(ProjectSpace* pProject)
	{
	}

	void MonoEditorExtension::OnProjectOpen(ProjectSpace* pProject)
	{
		std::filesystem::path solutionPath = pProject->RootPath();
		solutionPath = solutionPath.append(pProject->Name() + ".csproj");
		m_SolutionDocument = ticpp::Document(solutionPath.string());

		//if (std::filesystem::exists(solutionPath)) return;

		//ticpp::Declaration* decl = new ticpp::Declaration("1.0", "", "");
		//m_SolutionDocument.LinkEndChild(decl);

		ticpp::Element* root = new ticpp::Element("Project");
		m_SolutionDocument.LinkEndChild(root);
		root->SetAttribute("Sdk", "Microsoft.NET.Sdk");

		ticpp::Element* propertyGroup = new ticpp::Element("PropertyGroup");
		root->LinkEndChild(propertyGroup);

		ticpp::Element* outputType = new ticpp::Element("OutputType");
		propertyGroup->LinkEndChild(outputType);
		
		ticpp::Text* text = new ticpp::Text("Library");
		outputType->LinkEndChild(text);

		ticpp::Element* appDesignerFolder = new ticpp::Element("AppDesignerFolder");
		propertyGroup->LinkEndChild(appDesignerFolder);

		text = new ticpp::Text("Properties");
		appDesignerFolder->LinkEndChild(text);

		ticpp::Element* element = new ticpp::Element("RootNamespace");
		propertyGroup->LinkEndChild(element);

		text = new ticpp::Text(pProject->Name());
		element->LinkEndChild(text);

		element = new ticpp::Element("TargetFramework");
		propertyGroup->LinkEndChild(element);

		text = new ticpp::Text("netstandard2.1");
		element->LinkEndChild(text);

		element = new ticpp::Element("EnableDefaultCompileItems");
		propertyGroup->LinkEndChild(element);

		text = new ticpp::Text("false");
		element->LinkEndChild(text);

		ticpp::Element* itemGroup = new ticpp::Element("ItemGroup");
		root->LinkEndChild(itemGroup);

		ticpp::Element* reference = new ticpp::Element("Reference");
		itemGroup->LinkEndChild(reference);
		reference->SetAttribute("Include", "AppLogger");

		element = new ticpp::Element("HintPath");
		reference->LinkEndChild(element);

		text = new ticpp::Text("..\\..\\..\\Glorious\\TEST\\CSharpTest\\AppLogger\\bin\\Debug\\netcoreapp3.1\\AppLogger.dll");
		element->LinkEndChild(text);

		m_SolutionDocument.SaveFile();
	}
}

//<ItemGroup>
//	<Reference Include = "AppLogger">
//		<HintPath>..\..\..\Glorious\TEST\CSharpTest\AppLogger\bin\Debug\netcoreapp3.1\AppLogger.dll< / HintPath>
//	< / Reference>
//< / ItemGroup>