#pragma once

namespace Glory
{
	struct InputEvent;
	struct TextEvent;

	class IWindowInputOverrideHandler
	{
	public:
		virtual bool OnOverrideInputEvent(InputEvent& e) = 0;
		virtual bool OnOverrideTextEvent(TextEvent& e) = 0;
	};
}
