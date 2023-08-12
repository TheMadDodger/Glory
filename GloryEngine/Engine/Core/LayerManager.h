#pragma once
#include "Layer.h"
#include <vector>
#include <unordered_map>
#include "GloryContext.h"

#define LAYER_MANAGER GloryContext::GetLayerManager()

namespace Glory
{
	class LayerManager
	{
	public:
		static void AddLayer(const std::string& name);
		static void Load();

		static const Layer* GetLayerByName(const std::string& name);
		static std::string LayerMaskToString(const LayerMask& layerMask);

		static int GetLayerIndex(const Layer* pLayer);
		static void GetAllLayerNames(std::vector<std::string_view>& names);
		static const Layer* GetLayerAtIndex(int index);

		static const size_t LayerCount();

	private:
		static void CreateDefaultLayers();

	private:
		LayerManager();
		virtual ~LayerManager();

	private:
		friend class GloryContext;
		friend class Engine;
		std::vector<Layer> m_Layers;
		std::unordered_map<std::string, size_t> m_NameToLayer;
	};
}
