#include "LayerRef.h"
#include "LayerManager.h"
#include <Reflection.h>

namespace Glory
{
	LayerRef::LayerRef() : m_LayerIndex(0) {}
	LayerRef::LayerRef(size_t layerIndex) : m_LayerIndex(layerIndex) {}

	bool LayerRef::operator==(const LayerRef&& other)
	{
		return m_LayerIndex == other.m_LayerIndex;
	}

	bool LayerRef::operator==(LayerRef& other)
	{
		return m_LayerIndex == other.m_LayerIndex;
	}

	const Layer* LayerRef::Layer() const
	{
		return LayerManager::GetLayerAtIndex(m_LayerIndex);
	}

	const GloryReflect::TypeData* LayerRef::GetTypeData()
	{
		static const char* typeNameString = "LayerRef";
		static const size_t TYPE_HASH = std::hash<std::type_index>()(typeid(LayerRef));
		static const int NUM_ARGS = 0;
		static const GloryReflect::TypeData pTypeData = GloryReflect::TypeData(typeNameString, nullptr, TYPE_HASH, NUM_ARGS);
		return &pTypeData;
	}
}
