#pragma once
#include "Module.h"

namespace Glory
{
    class PhysicsModule : public Module
    {
	public:
		PhysicsModule();
		virtual ~PhysicsModule();

		virtual const std::type_info& GetModuleType() override;

	protected:
		virtual void Initialize() = 0;
		virtual void Cleanup() = 0;
    };
}
