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
		virtual void OnInitialize() override;
		virtual void OnCleanup() override;
		virtual void OnUpdate() override;

	private:
		std::hash<std::string> m_Hasher;
	};
}
