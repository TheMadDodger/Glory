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
		LayerRef();
		LayerRef(const std::string& name);

		bool operator==(const LayerRef&& other);
		bool operator==(LayerRef& other);

		std::string m_LayerName;
		const Layer* Layer() const;

		static const GloryReflect::TypeData* GetTypeData();
	};
}
