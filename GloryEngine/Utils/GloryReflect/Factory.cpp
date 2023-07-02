#include "Factory.h"

namespace GloryReflect
{
	FactoryBase::FactoryBase(uint32_t typeHash) : m_TypeHash(typeHash)
	{
	}
	FactoryBase::~FactoryBase()
	{
	}
}
