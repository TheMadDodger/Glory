#pragma once
#include "EntityComponentSerializer.h"
#include "Components.h"
#include <PropertySerializer.h>

namespace Glory
{
	class ScriptedComponentSerailizer : public PropertySerializer
	{
    public:
        ScriptedComponentSerailizer();
        virtual ~ScriptedComponentSerailizer();

    private:
        virtual void Serialize(const std::string& name, void* data, uint32_t typeHash, YAML::Emitter& out) override;
        virtual void Deserialize(void* data, uint32_t typeHash, YAML::Node& object) override;
	};
}
