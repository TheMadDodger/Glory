#include "Factory.h"

namespace Glory::Utils::Reflect
{
	FactoryBase::FactoryBase(uint32_t typeHash) : m_TypeHash(typeHash)
	{
	}
	FactoryBase::~FactoryBase()
	{
	}
}
