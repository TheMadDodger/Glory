#include "AnyConverter.h"
#include "ResourceType.h"
#include <glm/glm.hpp>

#define CONVERTER(x) { ResourceType::GetHash<x>(), new BasicTemplatedConverter<x>() }

namespace Glory
{
	std::map<size_t, AnyConverter*> AnyConverter::m_Converters = {
		CONVERTER(float),
		CONVERTER(int),
		CONVERTER(glm::vec2),
		CONVERTER(glm::vec3),
		CONVERTER(glm::vec4),
	};

	void AnyConverter::Convert(std::any& data, std::vector<char>& out)
	{
		const std::type_info& type = data.type();
		size_t typeHash = ResourceType::GetHash(type);
		auto it = m_Converters.find(typeHash);
		if (it == m_Converters.end()) return;
		m_Converters[typeHash]->OnConvert(data, out);
	}

	AnyConverter::AnyConverter(size_t typeHash)
		: m_TypeHash(typeHash) {}

	AnyConverter::~AnyConverter() {}
}
