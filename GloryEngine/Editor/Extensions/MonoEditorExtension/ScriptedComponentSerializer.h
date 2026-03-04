#pragma once
#include <Components.h>
#include <PropertySerializer.h>

namespace Glory
{
	class ScriptedComponentSerializer : public PropertySerializer
	{
    public:
        ScriptedComponentSerializer(Serializers* pSerializers);
        virtual ~ScriptedComponentSerializer();

    private:
        virtual void Serialize(void* data, uint32_t typeHash, Utils::NodeValueRef node) override;
        virtual void Deserialize(void* data, uint32_t typeHash, Utils::NodeValueRef node) override;
	};
}
