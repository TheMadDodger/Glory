#pragma once
#include "PropertySerializer.h"

namespace Glory
{
    class EnumPropertySerializer : public PropertySerializer
    {
    public:
        EnumPropertySerializer();
        virtual ~EnumPropertySerializer();

    private:
        virtual void Serialize(const GloryReflect::FieldData* pFieldData, void* data, YAML::Emitter& out) override;
        virtual void Deserialize(const GloryReflect::FieldData* pFieldData, void* data, YAML::Node& object) override;
    };
}
