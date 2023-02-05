#pragma once
#include <Object.h>
#include <typeindex>
#include "GloryEditor.h"
#include "ResourceType.h"

//#define REGISTERED_EDITORS EditorContext::GetEditors()->m_pRegisteredEditors
//#define ACTIVE_EDITORS EditorContext::GetEditors()->m_pActiveEditors

namespace Glory::Editor
{
	class Editor
	{
	public:
		template<class T>
		static void RegisterEditor()
		{
			Editor* pEditor = new T();
			RegisterEditor(pEditor);
		}

		static GLORY_EDITOR_API void RegisterEditor(Editor* pEditor);

		GLORY_EDITOR_API const size_t GetEditedType() const;

		virtual GLORY_EDITOR_API void Initialize();
		virtual bool OnGUI() = 0;

		static GLORY_EDITOR_API Editor* CreateEditor(Object* pObject);
		static GLORY_EDITOR_API size_t GetID(Editor* pEditor);
		virtual GLORY_EDITOR_API std::string Name();
		static GLORY_EDITOR_API std::vector<Editor*> FindEditors(UUID uuid);
		static GLORY_EDITOR_API void ReleaseEditor(Editor* pEditor);
		GLORY_EDITOR_API Object* GetTarget() const;

	protected:
		Object* m_pTarget;
		const size_t m_EditedType;

	protected:
		GLORY_EDITOR_API Editor(const size_t type);
		virtual GLORY_EDITOR_API ~Editor();

	private:
		friend class EditorApp;
		friend class InspectorWindow;
		static void Cleanup();
		virtual Editor* Create() = 0;

		static std::vector<Editor*> m_pRegisteredEditors;
		static std::vector<Editor*> m_pActiveEditors;
	};

	template<class TEditor, class TObject>
	class EditorTemplate : public Editor
	{
	public:
		EditorTemplate(size_t type) : Editor(type) {}
		EditorTemplate() : Editor(ResourceType::GetHash<TObject>()) {}

	private:
		virtual Editor* Create() override
		{
			return new TEditor();
		}

		// This function ensures that T is of type Object by causing a compiler error if it isn't
		void GLORY_EDITOR_API CompilerTest();
	};
}