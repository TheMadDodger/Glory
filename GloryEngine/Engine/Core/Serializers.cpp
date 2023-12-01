#include "Serializers.h"
#include "ResourceType.h"
#include "SerializedTypes.h"
#include "GScene.h"

namespace Glory
{
	/* This ensures legacy support by rerouting an old hash to a new one */
	const std::map<uint32_t, uint32_t> LegacyRehasher = {
		{ 3536807891, ResourceType::GetHash<GScene>() },
		{ 1002038608, SerializedType::ST_Object }
	};

	Serializers::Serializers()
	{
	}

	Serializers::~Serializers()
	{
	}
}
