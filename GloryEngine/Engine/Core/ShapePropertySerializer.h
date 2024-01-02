#pragma once
#include "PropertySerializer.h"

namespace Glory
{
    class ShapePropertySerializer : public PropertySerializer
    {
	public:
		ShapePropertySerializer(Serializers* pSerializers);
		virtual ~ShapePropertySerializer();

	private:
		void Serialize(const std::string& name, void* data, uint32_t typeHash, YAML::Emitter& out) override;
		void Deserialize(void* data, uint32_t typeHash, YAML::Node& object) override;
    };
}
