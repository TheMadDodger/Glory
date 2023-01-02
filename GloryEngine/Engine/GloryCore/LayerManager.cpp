#include "LayerManager.h"
#include "Game.h"
#include "Debug.h"
#include <yaml-cpp/yaml.h>
#include <fstream>
#include <filesystem>

namespace Glory
{
	void LayerManager::AddLayer(const std::string& name)
	{
		size_t index = LAYER_MANAGER->m_Layers.size();
		uint64_t mask = static_cast<uint64_t>(1) << index;
		LAYER_MANAGER->m_Layers.push_back(Layer(mask, name));
		LAYER_MANAGER->m_NameToLayer[name] = index;
	}

	void LayerManager::Load()
	{
		LAYER_MANAGER->m_Layers.clear();
		LAYER_MANAGER->m_NameToLayer.clear();

		std::filesystem::path layersPath = Game::GetSettingsPath();
		layersPath.append("Layers.yaml");

		if (!std::filesystem::exists(layersPath))
		{
			CreateDefaultLayers();
			return;
		}

		YAML::Node node = YAML::LoadFile(layersPath.string());
		if (!node.IsMap())
		{
			Debug::LogError("Could not load Layers.yaml!");
			return;
		}

		YAML::Node layersNode = node["Layers"];
		if (!layersNode.IsSequence())
		{
			Debug::LogError("Could not load Layers.yaml!");
			return;
		}

		for (size_t i = 0; i < layersNode.size(); i++)
		{
			YAML::Node layerNode = layersNode[i];
			YAML::Node nameNode = layerNode["Name"];

			if (!nameNode.IsDefined())
			{
				Debug::LogError("Could not load Layers.yaml!");
				return;
			}

			std::string name = nameNode.as<std::string>();
			AddLayer(name);
		}
	}

	const Layer* LayerManager::GetLayerByName(const std::string& name)
	{
		if (LAYER_MANAGER->m_NameToLayer.find(name) == LAYER_MANAGER->m_NameToLayer.end()) return nullptr;
		size_t index = LAYER_MANAGER->m_NameToLayer[name];
		return &LAYER_MANAGER->m_Layers[index];
	}

	std::string LayerManager::LayerMaskToString(const LayerMask& layerMask)
	{
		if (layerMask == 0) return "Default";

		std::string result = "";
		for (size_t i = 0; i < LAYER_MANAGER->m_Layers.size(); i++)
		{
			if ((layerMask & LAYER_MANAGER->m_Layers[i].m_Mask) == 0) continue;
			result += LAYER_MANAGER->m_Layers[i].m_Name + ',';
		}
		result.erase(result.end() - 1);
		return result;
	}

	int LayerManager::GetLayerIndex(const Layer* pLayer)
	{
		if (pLayer == nullptr) return -1;
		if (LAYER_MANAGER->m_NameToLayer.find(pLayer->m_Name) == LAYER_MANAGER->m_NameToLayer.end())
			return -1;
		return LAYER_MANAGER->m_NameToLayer[pLayer->m_Name];
	}

	void LayerManager::GetAllLayerNames(std::vector<std::string_view>& names)
	{
		names.push_back("Default"); // Layer 0
		for (size_t i = 0; i < LAYER_MANAGER->m_Layers.size(); i++)
		{
			names.push_back(LAYER_MANAGER->m_Layers[i].m_Name);
		}
	}

	const Layer* LayerManager::GetLayerAtIndex(int index)
	{
		if (index < 0 || (size_t)(index) >= LAYER_MANAGER->m_Layers.size()) return nullptr;
		return &LAYER_MANAGER->m_Layers[index];
	}

	const size_t LayerManager::LayerCount()
	{
		return LAYER_MANAGER->m_Layers.size();
	}

	void LayerManager::CreateDefaultLayers()
	{
		AddLayer("World");
		AddLayer("UI");
		AddLayer("Effects");
	}

	LayerManager::LayerManager() {}
	LayerManager::~LayerManager() {}
}
