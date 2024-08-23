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
        virtual void Serialize(void* data, uint32_t typeHash, Utils::NodeValueRef node) override;
        virtual void Deserialize(void* data, uint32_t typeHash, Utils::NodeValueRef node) override;
    };
}
