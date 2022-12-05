#include "LayerRef.h"
#include "LayerManager.h"
#include <Reflection.h>

namespace Glory
{
	LayerRef::LayerRef() : m_LayerName("") {}
	LayerRef::LayerRef(const std::string& name) : m_LayerName(name) {}

	bool LayerRef::operator==(const LayerRef&& other)
	{
		return m_LayerName == other.m_LayerName;
	}

	bool LayerRef::operator==(LayerRef& other)
	{
		return m_LayerName == other.m_LayerName;
	}

	const Layer* LayerRef::Layer() const
	{
		return LayerManager::GetLayerByName(m_LayerName);
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
