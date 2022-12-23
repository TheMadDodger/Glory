#pragma once
#include "Module.h"

namespace Glory
{
	class InputModule : public Module
	{
	public:
		InputModule();
		virtual ~InputModule();

		virtual const std::type_info& GetModuleType() override;

	protected:
		virtual void Initialize() = 0;
		virtual void Cleanup() = 0;
		virtual void Update() = 0;
	};
}
