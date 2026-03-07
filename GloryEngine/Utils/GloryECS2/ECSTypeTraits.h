#pragma once
#include <SparseTypeTraits.h>

namespace Glory::Utils::ECS
{
	template<typename Component>
	concept ComponentCompatible = std::is_class_v<Component> && requires {
		SparseCompatibleDense<Component>;
		Component();
	};

	template<typename Manager>
	concept IsComponentManager = std::is_base_of<IComponentManager, Manager>::value;
}