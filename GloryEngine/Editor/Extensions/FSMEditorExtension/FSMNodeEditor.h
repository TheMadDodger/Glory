#pragma once
#include <EditorWindow.h>

#include <glm/vec3.hpp>

namespace Glory::Editor
{
	class FSMEditor;

	class FSMNodeEditor : public EditorWindowTemplate<FSMNodeEditor>
	{
	public:
		FSMNodeEditor();
		virtual ~FSMNodeEditor();

	private:
		virtual void OnGUI() override;
		virtual void Update() override;
		virtual void Draw() override;

	private:
		FSMEditor* GetMainWindow();

	private:
		glm::vec3 m_PanPosition;
		glm::vec3 m_Zoom;
	};
}
