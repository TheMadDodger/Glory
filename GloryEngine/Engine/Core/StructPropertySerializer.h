#pragma once
#include "PropertySerializer.h"

namespace Glory
{
    class StructPropertySerializer : public PropertySerializer
    {
    public:
        StructPropertySerializer(Serializers* pSerializers);
        virtual ~StructPropertySerializer();

    private:
        virtual void Serialize(const std::string& name, void* data, uint32_t typeHash, YAML::Emitter& out) override;
        virtual void Deserialize(void* data, uint32_t typeHash, YAML::Node& object) override;
    };
}
