#include "RenderDocAPI.h"

#include <renderdoc_app.h>

#include <windows.h>

#include <cassert>

namespace Glory
{
	RENDERDOC_API_1_4_1* RDocAPIInstance = NULL;

	bool RenderDocAPI::InitializeRenderDoc()
	{
		// At init, on windows
		if (const HMODULE mod = GetModuleHandleA("renderdoc.dll"))
		{
			const pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
			const int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_4_1, (void**)&RDocAPIInstance);
			return ret == 1;
		}

		return false;
	}

	bool RenderDocAPI::IsInitialized()
	{
		return RDocAPIInstance != nullptr;
	}

	void RenderDocAPI::StartCapture()
	{
		assert(RDocAPIInstance != nullptr);
		RDocAPIInstance->StartFrameCapture(NULL, NULL);
	}

	void RenderDocAPI::EndCapture()
	{
		assert(RDocAPIInstance != nullptr);
		RDocAPIInstance->EndFrameCapture(NULL, NULL);
	}
}
