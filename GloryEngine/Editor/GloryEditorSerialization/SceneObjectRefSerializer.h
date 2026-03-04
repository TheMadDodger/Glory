#pragma once
#include "PropertySerializer.h"

namespace Glory
{
    class SceneObjectRefSerializer : public PropertySerializer
    {
    public:
        SceneObjectRefSerializer(Serializers* pSerializers) : PropertySerializer(pSerializers, SerializedType::ST_Object) {}

    private:
        virtual void Serialize(const std::vector<char>& buffer, uint32_t typeHash, size_t offset, size_t size, Utils::NodeValueRef node) override;
        virtual void Deserialize(std::vector<char>& buffer, size_t offset, size_t size, Utils::NodeValueRef node) override;
        virtual void Serialize(void* data, uint32_t typeHash, Utils::NodeValueRef node) override;
        virtual void Deserialize(void* data, uint32_t typeHash, Utils::NodeValueRef node) override;
    };
}
