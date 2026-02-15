#pragma once

namespace Glory
{
	class ICaptureHandler
	{
	public:
		virtual void StartCapture() = 0;
		virtual void EndCapture() = 0;
	};
}