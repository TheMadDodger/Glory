#include "ScenesModule.h"
#include "Engine.h"

namespace Glory
{
	const std::type_info& ScenesModule::GetModuleType()
	{
		return typeid(ScenesModule);
	}

	void ScenesModule::Paint()
	{
		m_pEngine->GetRendererModule()->StartFrame();
		OnPaint();
		m_pEngine->GetRendererModule()->EndFrame();
	}
}
