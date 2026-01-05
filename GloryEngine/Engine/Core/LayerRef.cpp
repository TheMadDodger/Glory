#include "LayerRef.h"
#include "LayerManager.h"
#include "SerializedTypes.h"
#include "ResourceType.h"

#include <Reflection.h>

namespace Glory
{
	LayerRef::LayerRef() : m_LayerIndex(0) {}
	LayerRef::LayerRef(uint32_t layerIndex) : m_LayerIndex(layerIndex) {}

	bool LayerRef::operator==(const LayerRef&& other)
	{
		return m_LayerIndex == other.m_LayerIndex;
	}

	bool LayerRef::operator==(LayerRef& other)
	{
		return m_LayerIndex == other.m_LayerIndex;
	}

	const Layer* LayerRef::Layer(LayerManager* pManager) const
	{
		return pManager->GetLayerAtIndex((int)(m_LayerIndex) - 1);
	}

	const TypeData* LayerRef::GetTypeData()
	{
		static const char* typeNameString = "LayerRef";
		static const uint32_t TYPE_HASH = ResourceTypes::GetHash<LayerRef>();
		static const int NUM_ARGS = 0;
		static const TypeData pTypeData = TypeData(typeNameString, nullptr, ST_Struct, TYPE_HASH, NUM_ARGS);
		return &pTypeData;
	}
}
