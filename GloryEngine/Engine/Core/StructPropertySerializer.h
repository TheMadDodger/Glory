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
        virtual void Serialize(void* data, uint32_t typeHash, Utils::NodeValueRef node) override;
        virtual void Deserialize(void* data, uint32_t typeHash, Utils::NodeValueRef node) override;
    };
}
