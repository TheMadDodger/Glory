#pragma once

namespace Glory
{
	class RenderDocAPI
	{
	public:
		static bool InitializeRenderDoc();
		static bool IsInitialized();
		static void StartCapture();
		static void EndCapture();
	};
}
