#pragma once
#include <EditorWindow.h>

namespace Glory::Editor
{
	class UIMainWindow;

	class UIElementInspector : public EditorWindowTemplate<UIElementInspector>
	{
	public:
		UIElementInspector();
		virtual ~UIElementInspector();

	private:
		virtual void OnGUI() override;
		UIMainWindow* GetMainWindow();
	};
}
