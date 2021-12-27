#include "LayerMask.h"
#include "LayerManager.h"
#include "Debug.h"

namespace Glory
{
	LayerMask& LayerMask::operator|=(const LayerMask& other)
	{
		m_Mask |= other.m_Mask;
		return *this;
	}

	LayerMask& LayerMask::operator ^=(const LayerMask& other)
	{
		m_Mask ^= other.m_Mask;
		return *this;
	}

	LayerMask LayerMask::operator&(const LayerMask& other)
	{
		LayerMask mask = *this;
		mask.m_Mask &= other.m_Mask;
		return mask;
	}

	LayerMask LayerMask::FromString(const std::string& names)
	{
		size_t currentIndex = 0;
		size_t nextIndex = 0;
		LayerMask layerMask(0);
		while (nextIndex != std::string::npos)
		{
			nextIndex = names.find(',', currentIndex + 1);
			std::string name = names.substr(currentIndex, nextIndex - currentIndex);
			const Layer* pLayer = LayerManager::GetLayerByName(name);
			if (pLayer == nullptr)
			{
				Debug::LogWarning("Layer with name " + name + " does not exist!");
				continue;
			}
			layerMask |= pLayer->m_Mask;
			currentIndex = nextIndex + 1;
		}
		return layerMask;
	}
}