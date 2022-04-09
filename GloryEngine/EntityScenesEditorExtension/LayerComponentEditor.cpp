#include "LayerComponentEditor.h"
#include <LayerManager.h>
#include <imgui.h>

namespace Glory::Editor
{
	LayerComponentEditor::LayerComponentEditor() {}
	LayerComponentEditor::~LayerComponentEditor() {}

	void LayerComponentEditor::Initialize()
	{
		EntityComponentEditor<LayerComponentEditor, LayerComponent>::Initialize();
		m_LayerOptions.clear();
		LayerManager::GetAllLayerNames(m_LayerOptions);
	}

	bool Glory::Editor::LayerComponentEditor::OnGUI()
	{
		LayerComponent& layer = GetTargetComponent();

		const Layer* pLayer = layer.m_pLayer;
		int index = LayerManager::GetLayerIndex(pLayer) + 1;
		int newIndex = index;

		ImGui::Text("Layer");
		ImGui::SameLine();
		if (ImGui::BeginCombo("##Layer", m_LayerOptions[index].data()))
		{
			for (size_t i = 0; i < m_LayerOptions.size(); i++)
			{
				bool selected = i == index;
				if (ImGui::Selectable(m_LayerOptions[i].data(), selected))
					newIndex = i;

				if (selected)
					ImGui::SetItemDefaultFocus();
			}

			ImGui::EndCombo();
		}

		if (newIndex == index) return false;
		index = newIndex - 1;
		pLayer = LayerManager::GetLayerAtIndex(index);
		layer.m_pLayer = pLayer;
		return true;
	}

	std::string LayerComponentEditor::Name()
	{
		return "Layer";
	}
}
