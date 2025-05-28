#pragma once
#include <EditorWindow.h>

namespace Glory::Editor
{
	class FSMEditor;

	class FSMDebugger : public EditorWindowTemplate<FSMDebugger>
	{
	public:
		FSMDebugger();
		virtual ~FSMDebugger();

	private:
		virtual void OnGUI() override;
		virtual void Update() override;
		virtual void Draw() override;

	private:
		FSMEditor* GetMainWindow();

	private:
		size_t m_LastFrameInstancesCount;
	};
}
