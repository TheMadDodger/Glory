#pragma once

namespace Glory
{
	struct InputEvent;

	class IWindowInputOverrideHandler
	{
	public:
		virtual bool OnOverrideInputEvent(InputEvent& e) = 0;
	};
}
