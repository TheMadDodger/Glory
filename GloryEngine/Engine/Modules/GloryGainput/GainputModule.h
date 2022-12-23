#pragma once
#include <InputModule.h>
#include <Glory.h>

namespace Glory
{
	class GainputModule : public InputModule
	{
	public:
		GLORY_API GainputModule();
		GLORY_API virtual ~GainputModule();

	private:
		virtual void Initialize() override;
		virtual void Cleanup() override;
		virtual void Update() override;
	};
}
