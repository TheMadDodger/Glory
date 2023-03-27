#pragma once
#include "PropertySerializer.h"

namespace Glory
{
    class StructPropertySerializer : public PropertySerializer
    {
    public:
        StructPropertySerializer();
        virtual ~StructPropertySerializer();

    private:
        virtual void Serialize(const GloryReflect::FieldData* pFieldData, void* data, YAML::Emitter& out) override;
        virtual void Deserialize(const GloryReflect::FieldData* pFieldData, void* data, YAML::Node& object) override;
    };
}
