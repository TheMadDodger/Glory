#pragma once
#include <EditorWindow.h>
#include <AudioData.h>

namespace Glory
{
	class UIDocumentData;
	class UIDocument;
}

namespace Glory::Editor
{
	class UIEditor : public EditorWindowTemplate<UIEditor>
	{
	public:
		UIEditor();
		virtual ~UIEditor();

		void SetDocument(UUID documentID);

	private:
		virtual void OnGUI() override;
		virtual void Update() override;
		virtual void Draw() override;

	private:
		UUID m_EditingDocument;
		UIDocument* m_pDocument;
		glm::uvec2 m_Resolution;
	};
}
