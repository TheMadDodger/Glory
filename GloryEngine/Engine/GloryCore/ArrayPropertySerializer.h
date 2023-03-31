#pragma once
#include "PropertySerializer.h"

namespace Glory
{
    class ArrayPropertySerializer : public PropertySerializer
    {
    public:
        ArrayPropertySerializer();
        virtual ~ArrayPropertySerializer();

    private:
        virtual void Serialize(const GloryReflect::FieldData* pFieldData, void* data, YAML::Emitter& out) override;
        virtual void Deserialize(void* data, uint32_t typeHash, YAML::Node& object) override;
    };
}
