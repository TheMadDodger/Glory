#pragma once
#include "PropertySerializer.h"

namespace Glory
{
    class AssetReferencePropertySerializer : public PropertySerializer
    {
    public:
        AssetReferencePropertySerializer(Serializers* pSerializers);
        virtual ~AssetReferencePropertySerializer();

    private:
        virtual void Serialize(const std::string& name, void* data, uint32_t typeHash, YAML::Emitter& out) override;
        virtual void Deserialize(void* data, uint32_t typeHash, YAML::Node& object) override;
    };
}
