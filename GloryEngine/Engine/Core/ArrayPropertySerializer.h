#pragma once
#include "PropertySerializer.h"

namespace Glory
{
    class ArrayPropertySerializer : public PropertySerializer
    {
    public:
        ArrayPropertySerializer(Serializers* pSerializers);
        virtual ~ArrayPropertySerializer();

    private:
        virtual void Serialize(void* data, uint32_t typeHash, Utils::NodeValueRef node) override;
        virtual void Deserialize(void* data, uint32_t typeHash, Utils::NodeValueRef node) override;
    };
}
