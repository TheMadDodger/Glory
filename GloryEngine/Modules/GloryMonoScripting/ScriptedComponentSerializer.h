#pragma once
#include "Components.h"
#include "PropertySerializer.h"

namespace Glory
{
	class ScriptedComponentSerailizer : public PropertySerializer
	{
    public:
        ScriptedComponentSerailizer(Serializers* pSerializers);
        virtual ~ScriptedComponentSerailizer();

    private:
        virtual void Serialize(void* data, uint32_t typeHash, Utils::NodeValueRef node) override;
        virtual void Deserialize(void* data, uint32_t typeHash, Utils::NodeValueRef node) override;
	};
}
