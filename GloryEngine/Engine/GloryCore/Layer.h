#pragma once
#include "LayerMask.h"
#include <string>

namespace Glory
{
	struct Layer
	{
	public:
		const LayerMask m_Mask;
		const std::string m_Name;
		virtual ~Layer();

	private:
		friend class LayerManager;
		Layer(const LayerMask& layerMask, const std::string& name);
	};
}
