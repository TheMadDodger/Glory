#pragma once
#include <EditorWindow.h>
#include <AudioData.h>

namespace Glory::Editor
{
	class UIEditor : public EditorWindowTemplate<UIEditor>
	{
	public:
		UIEditor();
		virtual ~UIEditor();

	private:
		virtual void OnGUI() override;
		virtual void Update() override;
		virtual void Draw() override;
	};
}
