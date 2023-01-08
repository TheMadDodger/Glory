#pragma once
#include <string>

namespace GloryReflect
{
	struct TypeData;
}

namespace Glory
{
	struct Layer;

	struct LayerRef
	{
	public:
		LayerRef();
		LayerRef(size_t layerIndex);

		size_t m_LayerIndex;

	public:
		bool operator==(const LayerRef&& other);
		bool operator==(LayerRef& other);

		const Layer* Layer() const;

		static const GloryReflect::TypeData* GetTypeData();

	};
}
