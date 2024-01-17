#include "LayerManager.h"
#include "Engine.h"
#include "AssetDatabase.h"
#include "Debug.h"

#include <yaml-cpp/yaml.h>
#include <fstream>
#include <filesystem>

namespace Glory
{
	void LayerManager::AddLayer(const std::string& name)
	{
		size_t index = m_Layers.size();
		uint64_t mask = static_cast<uint64_t>(1) << index;
		m_Layers.push_back(Layer(mask, name));
		m_NameToLayer[name] = index;
	}

	void LayerManager::Load()
	{
		m_Layers.clear();
		m_NameToLayer.clear();

		std::filesystem::path layersPath = m_pEngine->GetAssetDatabase().GetSettingsPath();
		layersPath.append("Layers.yaml");

		if (!std::filesystem::exists(layersPath))
		{
			CreateDefaultLayers();
			return;
		}

		YAML::Node node = YAML::LoadFile(layersPath.string());
		if (!node.IsMap())
		{
			m_pEngine->GetDebug().LogError("Could not load Layers.yaml!");
			return;
		}

		YAML::Node layersNode = node["Layers"];
		if (!layersNode.IsSequence())
		{
			m_pEngine->GetDebug().LogError("Could not load Layers.yaml!");
			return;
		}

		for (size_t i = 0; i < layersNode.size(); i++)
		{
			YAML::Node layerNode = layersNode[i];
			YAML::Node nameNode = layerNode["Name"];

			if (!nameNode.IsDefined())
			{
				m_pEngine->GetDebug().LogError("Could not load Layers.yaml!");
				return;
			}

			std::string name = nameNode.as<std::string>();
			AddLayer(name);
		}
	}

	const Layer* LayerManager::GetLayerByName(const std::string& name)
	{
		if (m_NameToLayer.find(name) == m_NameToLayer.end()) return nullptr;
		size_t index = m_NameToLayer[name];
		return &m_Layers[index];
	}

	std::string LayerManager::LayerMaskToString(const LayerMask& layerMask)
	{
		if (layerMask == 0) return "Default";

		std::string result = "";
		for (size_t i = 0; i < m_Layers.size(); i++)
		{
			if ((layerMask & m_Layers[i].m_Mask) == 0) continue;
			result += m_Layers[i].m_Name + ',';
		}
		result.erase(result.end() - 1);
		return result;
	}

	int LayerManager::GetLayerIndex(const Layer* pLayer)
	{
		if (pLayer == nullptr) return -1;
		if (m_NameToLayer.find(pLayer->m_Name) == m_NameToLayer.end())
			return -1;
		return (int)m_NameToLayer[pLayer->m_Name];
	}

	void LayerManager::GetAllLayerNames(std::vector<std::string_view>& names)
	{
		names.push_back("Default"); // Layer 0
		for (size_t i = 0; i < m_Layers.size(); i++)
		{
			names.push_back(m_Layers[i].m_Name);
		}
	}

	const Layer* LayerManager::GetLayerAtIndex(int index)
	{
		if (index < 0 || (size_t)(index) >= m_Layers.size()) return nullptr;
		return &m_Layers[index];
	}

	const size_t LayerManager::LayerCount()
	{
		return m_Layers.size();
	}

	void LayerManager::CreateDefaultLayers()
	{
		AddLayer("World");
		AddLayer("UI");
		AddLayer("Effects");
	}

	LayerManager::LayerManager(Engine* pEngine): m_pEngine(pEngine) {}
	LayerManager::~LayerManager() { m_pEngine = nullptr; }
}
