#include "ProjectSettings.h"
#include "EditorUI.h"
#include "EditorApplication.h"

#include <LayerManager.h>
#include <Undo.h>
#include <BinaryStream.h>

namespace Glory::Editor
{
	const size_t LAYER_BUFFER_SIZE = 256;
	char LAYER_BUFFER[LAYER_BUFFER_SIZE] = "\0";

	bool LayerSettings::OnGui()
	{
		Utils::NodeValueRef layersNode = RootValue()["Layers"];

		const size_t count = layersNode.Size();
		for (size_t i = 0; i < count; ++i)
		{
			Utils::NodeValueRef layerNode = layersNode[i];
			Utils::NodeValueRef layerNameNode = layerNode["Name"];

			const std::string label = "Layer " + std::to_string(i);
			const std::string layerName = layerNameNode.As<std::string>();
			strcpy(LAYER_BUFFER, layerName.data());
			if (EditorUI::InputText(m_YAMLFile, layerNameNode.Path(), ImGuiInputTextFlags_EnterReturnsTrue))
			{
				SaveSettings(ProjectSpace::GetOpenProject());
				EditorApplication::GetInstance()->GetEngine()->GetLayerManager().Load();
			}
		}

		LAYER_BUFFER[0] = '\0';
		if (EditorUI::InputText("New Layer", LAYER_BUFFER, LAYER_BUFFER_SIZE, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			std::string name = std::string(LAYER_BUFFER);
			if (!name.empty())
			{
				Undo::StartRecord("Add Layer");
				YAML::Node oldValue = YAML::Node(YAML::NodeType::Null);
				YAML::Node newValue = YAML::Node(YAML::NodeType::Map);
				newValue["Name"] = std::string(LAYER_BUFFER);
				layersNode.PushBack(newValue);

				Utils::NodeValueRef layerNode = layersNode[count];
				Undo::YAMLEdit(m_YAMLFile, layerNode.Path(), oldValue, newValue);
				Undo::StopRecord();
				SaveSettings(ProjectSpace::GetOpenProject());
				EditorApplication::GetInstance()->GetEngine()->GetLayerManager().Load();
			}
		}
		return false;
	}

	void LayerSettings::OnSettingsLoaded()
	{
		EditorApplication::GetInstance()->GetEngine()->GetLayerManager().Load();
	}

	void LayerSettings::OnCompile(const std::filesystem::path& path)
	{
		std::filesystem::path finalPath = path;
		finalPath.replace_filename("Layers.dat");
		BinaryFileStream file{ finalPath };
		BinaryStream* stream = &file;
		stream->Write(CoreVersion);

		Utils::NodeValueRef layersNode = RootValue()["Layers"];

		const size_t count = layersNode.Size();
		for (size_t i = 0; i < count; ++i)
		{
			Utils::NodeValueRef layerNode = layersNode[i];
			Utils::NodeValueRef layerNameNode = layerNode["Name"];
			const std::string layerName = layerNameNode.As<std::string>();
			stream->Write(layerName);
		}
	}
}
