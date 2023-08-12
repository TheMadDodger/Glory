#pragma once
#include "PropertySerializer.h"

namespace Glory
{
    class SceneObjectRefSerializer : public PropertySerializer
    {
    public:
        SceneObjectRefSerializer() : PropertySerializer(SerializedType::ST_Object) {}

    private:
        virtual void Serialize(const std::string& name, const std::vector<char>& buffer, uint32_t typeHash, size_t offset, size_t size, YAML::Emitter& out) override;
        virtual void Deserialize(std::vector<char>& buffer, size_t offset, size_t size, YAML::Node& object) override;
        virtual void Serialize(const std::string& name, void* data, uint32_t typeHash, YAML::Emitter& out) override;
        virtual void Deserialize(void* data, uint32_t typeHash, YAML::Node& object) override;
    };
}
