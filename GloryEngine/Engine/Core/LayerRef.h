#pragma once
#include <string>

namespace Glory::Utils::Reflect
{
	struct TypeData;
}

namespace Glory
{
	class LayerManager;
	struct Layer;

	struct LayerRef
	{
	public:
		LayerRef();
		LayerRef(uint32_t layerIndex);

		uint32_t m_LayerIndex;

	public:
		bool operator==(const LayerRef&& other);
		bool operator==(LayerRef& other);

		const Layer* Layer(LayerManager* pManager) const;

		static const Glory::Utils::Reflect::TypeData* GetTypeData();

	};
}
