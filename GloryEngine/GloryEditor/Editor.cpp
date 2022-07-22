#include <algorithm>
#include "Editor.h"

namespace Glory::Editor
{
	void Editor::Initialize() {}

	Editor* Editor::CreateEditor(Object* pObject)
	{
		for (size_t i = 0; i < pObject->TypeCount(); i++)
		{
			std::type_index type = typeid(Object);
			if (!pObject->GetType(i, type)) continue;

			auto it = std::find_if(REGISTERED_EDITORS.begin(), REGISTERED_EDITORS.end(), [&](Editor* pEditor)
			{
				std::type_index editorType = pEditor->GetEditedType();
				return editorType == type;
			});

			if (it == REGISTERED_EDITORS.end()) continue;
			Editor* pEditor = *it;
			Editor* newEditor = pEditor->Create();
			newEditor->m_pTarget = pObject;
			newEditor->Initialize();
			ACTIVE_EDITORS.push_back(newEditor);
			return newEditor;
		}

		return nullptr;
	}

	size_t Editor::GetID(Editor* pEditor)
	{
		for (size_t i = 0; i < ACTIVE_EDITORS.size(); i++)
		{
			if (ACTIVE_EDITORS[i] == pEditor) return i;
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
		std::for_each(REGISTERED_EDITORS.begin(), REGISTERED_EDITORS.end(), [](Editor* pEditor) { delete pEditor; });
		std::for_each(ACTIVE_EDITORS.begin(), ACTIVE_EDITORS.end(), [](Editor* pEditor) { delete pEditor; });
		REGISTERED_EDITORS.clear();
		ACTIVE_EDITORS.clear();
	}

	template<class TEditor, class TObject>
	inline void EditorTemplate<TEditor, TObject>::CompilerTest()
	{
		Editor* pObject = new TEditor();
		Object* pObject = new TObject();
	}
}