#include "GLORY_YAML.h"

namespace YAML
{
	Emitter& operator<<(Emitter& out, const Glory::LayerMask& mask)
	{
		out << mask.m_Mask;
		return out;
	}
}
