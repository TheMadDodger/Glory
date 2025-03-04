#pragma once
#include <EditorWindow.h>

namespace Glory::Editor
{
	class UIMainWindow;

    class AddUIElementWindow : public EditorWindowTemplate<AddUIElementWindow>
    {
	public:
		AddUIElementWindow();
		virtual ~AddUIElementWindow();

	private:
		virtual void OnGUI() override;
		UIMainWindow* GetMainWindow();
    };
}
