#pragma once
#include "PropertySerializer.h"

namespace Glory
{
    class ArrayPropertySerializers : public PropertySerializer
    {
    public:
        ArrayPropertySerializers();
        virtual ~ArrayPropertySerializers();

    private:
        virtual void Serialize(const SerializedProperty* serializedProperty, YAML::Emitter& out) override;
        virtual void Deserialize(const SerializedProperty* serializedProperty, YAML::Node& object) override;
    };
}
