#include "ProjectSettings.h"
#include "EditorUI.h"
#include <LayerManager.h>

namespace Glory::Editor
{
	const size_t LAYER_BUFFER_SIZE = 256;
	char LAYER_BUFFER[LAYER_BUFFER_SIZE] = "\0";

	void LayerSettings::OnGui()
	{
		YAML::Node layersNode = m_SettingsNode["Layers"];

		for (size_t i = 0; i < LayerManager::LayerCount(); i++)
		{
			YAML::Node layerNode = layersNode[i];
			YAML::Node layerNameNode = layerNode["Name"];

			const std::string label = "Layer " + std::to_string(i);
			const Layer* pLayer = LayerManager::GetLayerAtIndex(i);

			strcpy(LAYER_BUFFER, pLayer->m_Name.data());
			if (EditorUI::InputText(label.data(), LAYER_BUFFER, LAYER_BUFFER_SIZE, ImGuiInputTextFlags_EnterReturnsTrue))
			{
				layerNameNode = std::string(LAYER_BUFFER);
				SaveSettings(ProjectSpace::GetOpenProject());
				LayerManager::Load();
			}
		}

		LAYER_BUFFER[0] = '\0';
		if (EditorUI::InputText("New Layer", LAYER_BUFFER, LAYER_BUFFER_SIZE, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			std::string name = std::string(LAYER_BUFFER);
			if (!name.empty())
			{
				YAML::Node newLayerNode{ YAML::NodeType::Map };
				newLayerNode["Name"] = std::string(LAYER_BUFFER);

				layersNode.push_back(newLayerNode);
				SaveSettings(ProjectSpace::GetOpenProject());
				LayerManager::Load();
			}
		}
	}
}
