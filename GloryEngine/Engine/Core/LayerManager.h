#pragma once
#include "Layer.h"

#include <vector>
#include <unordered_map>

#define LAYER_MANAGER GloryContext::GetLayerManager()

namespace Glory
{
	class IEngine;

	class LayerManager
	{
	public:
		LayerManager(IEngine* pEngine);
		virtual ~LayerManager();

		void AddLayer(const std::string& name);
		void Load();

		const Layer* GetLayerByName(const std::string& name);
		std::string LayerMaskToString(const LayerMask& layerMask);

		int GetLayerIndex(const Layer* pLayer);
		void GetAllLayerNames(std::vector<std::string_view>& names);
		const Layer* GetLayerAtIndex(int index);

		const size_t LayerCount();

	private:
		void CreateDefaultLayers();

	private:
		friend class GloryContext;
		friend class IEngine;
		IEngine* m_pEngine;
		std::vector<Layer> m_Layers;
		std::unordered_map<std::string, size_t> m_NameToLayer;
	};
}
