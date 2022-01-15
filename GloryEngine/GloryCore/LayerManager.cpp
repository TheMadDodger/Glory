#include "LayerManager.h"
#include "Game.h"
#include "Debug.h"
#include <yaml-cpp/yaml.h>
#include <fstream>
#include <filesystem>

namespace Glory
{
	std::vector<Layer> LayerManager::m_Layers;
	std::unordered_map<std::string, size_t> LayerManager::m_NameToLayer;

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

		std::filesystem::path layersPath = Game::GetAssetPath();
		layersPath = layersPath.parent_path();
		layersPath.append("Layers.yaml");

		if (!std::filesystem::exists(layersPath))
		{
			CreateDefaultLayers();
			Save();
			return;
		}

		YAML::Node node = YAML::LoadFile(layersPath.string());
		if (!node.IsSequence())
		{
			Debug::LogError("Could not load Layers.yaml!");
			return;
		}

		for (size_t i = 0; i < node.size(); i++)
		{
			YAML::Node layerNode = node[i];
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

	void LayerManager::Save()
	{
		YAML::Emitter emitter;
		emitter << YAML::BeginSeq;
		
		for (size_t i = 0; i < m_Layers.size(); i++)
		{
			Layer* pLayer = &m_Layers[i];
			emitter << YAML::BeginMap;
			emitter << YAML::Key << "Name";
			emitter << YAML::Value << pLayer->m_Name;
			emitter << YAML::EndMap;
		}

		emitter << YAML::EndSeq;

		std::filesystem::path layersPath = Game::GetAssetPath();
		layersPath = layersPath.parent_path();
		layersPath.append("Layers.yaml");

		std::ofstream outStream(layersPath.string());
		outStream << emitter.c_str();
		outStream.close();
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
		return m_NameToLayer[pLayer->m_Name];
	}

	void LayerManager::GetAllLayerNames(std::vector<std::string>& names)
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

	void LayerManager::CreateDefaultLayers()
	{
		AddLayer("World");
		AddLayer("UI");
		AddLayer("Effects");
	}

	LayerManager::LayerManager() {}
	LayerManager::~LayerManager() {}
}
