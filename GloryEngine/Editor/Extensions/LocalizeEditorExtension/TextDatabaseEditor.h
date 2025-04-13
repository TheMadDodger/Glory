#pragma once
#include <EditorWindow.h>

namespace Glory::Editor
{
	class FSMEditor;

	class TextDatabaseEditor : public EditorWindowTemplate<TextDatabaseEditor>
	{
	public:
		TextDatabaseEditor();
		virtual ~TextDatabaseEditor();

	private:
		virtual void OnGUI() override;
		virtual void Update() override;
		virtual void Draw() override;
	};
}
