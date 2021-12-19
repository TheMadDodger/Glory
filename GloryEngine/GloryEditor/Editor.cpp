#include <algorithm>
#include "Editor.h"

namespace Glory::Editor
{
	std::vector<Editor*> Editor::m_pRegisteredEditors = std::vector<Editor*>();
	std::vector<Editor*> Editor::m_pActiveEditors = std::vector<Editor*>();

	void Editor::Initialize() {}

	Editor* Editor::CreateEditor(Object* pObject)
	{
		for (size_t i = 0; i < pObject->TypeCount(); i++)
		{
			std::type_index type = typeid(Object);
			if (!pObject->GetType(i, type)) continue;

			auto it = std::find_if(m_pRegisteredEditors.begin(), m_pRegisteredEditors.end(), [&](Editor* pEditor)
			{
				std::type_index editorType = pEditor->GetEditedType();
				return editorType == type;
			});

			if (it == m_pRegisteredEditors.end()) continue;
			Editor* pEditor = *it;
			Editor* newEditor = pEditor->Create();
			newEditor->m_pTarget = pObject;
			newEditor->Initialize();
			m_pActiveEditors.push_back(newEditor);
			return newEditor;
		}

		return nullptr;
	}

	size_t Editor::GetID(Editor* pEditor)
	{
		for (size_t i = 0; i < m_pActiveEditors.size(); i++)
		{
			if (m_pActiveEditors[i] == pEditor) return i;
		}
		return 0;
	}

	std::string Editor::Name()
	{
		return "Editor";
	}

	Editor::Editor() : m_pTarget(nullptr)
	{
	}

	Editor::~Editor()
	{
	}

	void Editor::Cleanup()
	{
		std::for_each(m_pRegisteredEditors.begin(), m_pRegisteredEditors.end(), [](Editor* pEditor) { delete pEditor; });
		std::for_each(m_pActiveEditors.begin(), m_pActiveEditors.end(), [](Editor* pEditor) { delete pEditor; });
		m_pRegisteredEditors.clear();
		m_pActiveEditors.clear();
	}

	template<class TEditor, class TObject>
	inline void EditorTemplate<TEditor, TObject>::CompilerTest()
	{
		Editor* pObject = new TEditor();
		Object* pObject = new TObject();
	}
}