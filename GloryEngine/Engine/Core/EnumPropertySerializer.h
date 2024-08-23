#pragma once
#include "PropertySerializer.h"

namespace Glory
{
    class EnumPropertySerializer : public PropertySerializer
    {
    public:
        EnumPropertySerializer(Serializers* pSerializers);
        virtual ~EnumPropertySerializer();

    private:
        virtual void Serialize(void* data, uint32_t typeHash, Utils::NodeValueRef node) override;
        virtual void Deserialize(void* data, uint32_t typeHash, Utils::NodeValueRef node) override;
    };
}
