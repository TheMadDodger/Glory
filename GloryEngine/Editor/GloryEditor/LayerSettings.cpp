#include "ProjectSettings.h"
#include "EditorUI.h"
#include <LayerManager.h>

namespace Glory::Editor
{
	const size_t LAYER_BUFFER_SIZE = 256;
	char LAYER_BUFFER[LAYER_BUFFER_SIZE] = "\0";

	bool LayerSettings::OnGui()
	{
		NodeValueRef layersNode = RootValue()["Layers"];

		size_t count = layersNode.Size();
		for (size_t i = 0; i < count; ++i)
		{
			NodeValueRef layerNode = layersNode[i];
			NodeValueRef layerNameNode = layerNode["Name"];

			const std::string label = "Layer " + std::to_string(i);
			const std::string layerName = layerNameNode.As<std::string>();
			strcpy(LAYER_BUFFER, layerName.data());
			if (EditorUI::InputText(label.data(), LAYER_BUFFER, LAYER_BUFFER_SIZE, ImGuiInputTextFlags_EnterReturnsTrue))
			{
				layerNameNode.Set(std::string(LAYER_BUFFER));
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

				layersNode.PushBack(newLayerNode);
				SaveSettings(ProjectSpace::GetOpenProject());
				LayerManager::Load();
			}
		}
		return false;
	}

	void LayerSettings::OnSettingsLoaded()
	{
		LayerManager::Load();
	}
}
