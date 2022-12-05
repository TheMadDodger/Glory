#include "Layer.h"

namespace Glory
{
	Layer::Layer(const LayerMask& layerMask, const std::string& name) : m_Mask(layerMask), m_Name(name) {}
	Layer::~Layer() {}
}