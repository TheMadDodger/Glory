#pragma once
#include "GloryContext.h"

#ifndef SERIALIZERS
#define SERIALIZERS Glory::GloryContext::GetSerializers()->m_pRegisteredSerializers
#endif

namespace Glory
{
	class PropertySerializer;

	class Serializers
	{
	public:
		Serializers();
		virtual ~Serializers();

	private:
		friend class PropertySerializer;
		std::vector<PropertySerializer*> m_pRegisteredPropertySerializers;
	};
}
