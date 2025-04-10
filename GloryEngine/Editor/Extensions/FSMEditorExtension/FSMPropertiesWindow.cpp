#include "FSMPropertiesWindow.h"
#include "FSMEditor.h"

#include <EditableResource.h>
#include <EditorResourceManager.h>
#include <EditorUI.h>
#include <EditorAssetDatabase.h>

#include <Engine.h>
#include <FSM.h>

namespace Glory::Editor
{
	FSMPropertiesWindow::FSMPropertiesWindow() : EditorWindowTemplate("Properties", 600.0f, 600.0f)
	{
	}

	FSMPropertiesWindow::~FSMPropertiesWindow()
	{
	}

	void FSMPropertiesWindow::OnGUI()
	{
		const UUID fsmID = GetMainWindow()->CurrentFSMID();

		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		EditorResourceManager& resources = EditorApplication::GetInstance()->GetResourceManager();
		EditableResource* pResource = resources.GetEditableResource(fsmID);
		YAMLResource<FSMData>* pDocument = static_cast<YAMLResource<FSMData>*>(pResource);
		Utils::YAMLFileRef& file = **pDocument;

		auto properties = file["Properties"];

		bool change = false;

		for (auto iter = properties.Begin(); iter != properties.End(); ++iter)
		{
			const std::string key = *iter;
			auto prop = properties[key];
			auto type = prop["Type"];
			auto name = prop["Name"];

			ImGui::PushID(key.data());
			ImGui::BeginChild(key.data(), ImVec2{0.0f, 68.0f}, true, ImGuiWindowFlags_AlwaysAutoResize);
			change |= EditorUI::InputText(file, name.Path());
			change |= EditorUI::InputEnum<FSMPropertyType>(file, type.Path());
			ImGui::EndChild();
			ImGui::PopID();
		}

		if (change)
			EditorAssetDatabase::SetAssetDirty(fsmID);
	}

	void FSMPropertiesWindow::Update()
	{
	}

	void FSMPropertiesWindow::Draw()
	{
	}

	FSMEditor* FSMPropertiesWindow::GetMainWindow()
	{
		return static_cast<FSMEditor*>(m_pOwner);
	}
}
