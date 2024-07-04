#include <algorithm>
#include "Editor.h"

namespace Glory::Editor
{
	std::vector<Editor*> Editor::m_pRegisteredEditors;
	std::vector<Editor*> Editor::m_pActiveEditors;
	bool Editor::m_CleaningUp = false;

	void Editor::RegisterEditor(Editor* pEditor)
	{
		m_pRegisteredEditors.push_back(pEditor);
	}

	const size_t Editor::GetEditedType() const
	{
		return m_EditedType;
	}

	void Editor::Initialize() {}

	Editor* Editor::CreateEditor(Object* pObject)
	{
		for (size_t i = 0; i < pObject->TypeCount(); i++)
		{
			std::type_index type = typeid(Object);
			if (!pObject->GetType(i, type)) continue;
			const uint32_t typeHash = ResourceTypes::GetHash(type);

			auto it = std::find_if(m_pRegisteredEditors.begin(), m_pRegisteredEditors.end(), [&](Editor* pEditor)
			{
				const size_t editorType = pEditor->GetEditedType();
				return editorType == typeHash;
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

	std::vector<Editor*> Editor::FindEditors(UUID uuid)
	{
		std::vector<Editor*> result;
		std::for_each(m_pActiveEditors.begin(), m_pActiveEditors.end(), [&](Editor* pEditor)
		{
			if (pEditor->m_pTarget->GetUUID() != uuid) return;
			result.push_back(pEditor);
		});
		return result;
	}

	void Editor::ReleaseEditor(Editor* pEditor)
	{
		if (m_CleaningUp) return;
		auto it = std::find(m_pActiveEditors.begin(), m_pActiveEditors.end(), pEditor);
		if (it == m_pActiveEditors.end()) return;
		m_pActiveEditors.erase(it);
		delete pEditor;
	}

	Object* Editor::GetTarget() const
	{
		return m_pTarget;
	}

	void Editor::SetTarget(Object* pTarget)
	{
		/** @todo Check if type of target matches the edited type */
		m_pTarget = pTarget;
	}

	Editor::Editor(const uint32_t type) : m_pTarget(nullptr), m_EditedType(type)
	{
	}

	Editor::~Editor()
	{
	}

	void Editor::Cleanup()
	{
		m_CleaningUp = true;
		std::for_each(m_pRegisteredEditors.begin(), m_pRegisteredEditors.end(), [](Editor* pEditor) { delete pEditor; });
		std::for_each(m_pActiveEditors.begin(), m_pActiveEditors.end(), [](Editor* pEditor) { delete pEditor; });
		m_pRegisteredEditors.clear();
		m_pActiveEditors.clear();
		m_CleaningUp = false;
	}

	template<class TEditor, class TObject>
	inline void EditorTemplate<TEditor, TObject>::CompilerTest()
	{
		Editor* pObject = new TEditor();
		Object* pObject = new TObject();
	}
}