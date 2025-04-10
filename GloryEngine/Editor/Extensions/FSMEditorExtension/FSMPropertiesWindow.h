#pragma once
#include <EditorWindow.h>

namespace Glory::Editor
{
	class FSMEditor;

    class FSMPropertiesWindow : public EditorWindowTemplate<FSMPropertiesWindow>
    {
	public:
		FSMPropertiesWindow();
		virtual ~FSMPropertiesWindow();

	private:
		virtual void OnGUI() override;
		virtual void Update() override;
		virtual void Draw() override;

	private:
		FSMEditor* GetMainWindow();
    };
}
