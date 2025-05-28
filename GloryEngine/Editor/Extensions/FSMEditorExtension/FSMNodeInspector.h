#pragma once
#include <EditorWindow.h>

namespace Glory::Editor
{
	class FSMEditor;

	class FSMNodeInspector : public EditorWindowTemplate<FSMNodeInspector>
	{
	public:
		FSMNodeInspector();
		virtual ~FSMNodeInspector();

	private:
		virtual void OnGUI() override;
		virtual void Update() override;
		virtual void Draw() override;

	private:
		FSMEditor* GetMainWindow();

	private:
		UUID m_LastFrameSelectedNode;
	};
}
