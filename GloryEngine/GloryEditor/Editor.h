#pragma once
#include <Object.h>
#include <typeindex>
#include "GloryEditor.h"
#include "EditorContext.h"

#define REGISTERED_EDITORS EditorContext::GetEditors()->m_pRegisteredEditors
#define ACTIVE_EDITORS EditorContext::GetEditors()->m_pActiveEditors

namespace Glory::Editor
{
	class Editor
	{
	public:
		template<class T>
		static void RegisterEditor()
		{
			Editor* pEditor = new T();
			REGISTERED_EDITORS.push_back(pEditor);
		}

		virtual const std::type_index& GetEditedType() = 0;
		virtual void Initialize();
		virtual bool OnGUI() = 0;

		static Editor* CreateEditor(Object* pObject);

		static size_t GetID(Editor* pEditor);

		virtual std::string Name();

	protected:
		Object* m_pTarget;

	protected:
		Editor();
		virtual ~Editor();

	private:
		friend class EditorApp;
		friend class InspectorWindow;
		static void Cleanup();
		virtual Editor* Create() = 0;
	};

	template<class TEditor, class TObject>
	class EditorTemplate : public Editor
	{
	public:
		virtual const std::type_index& GetEditedType() override
		{
			return typeid(TObject);
		}

	private:
		virtual Editor* Create() override
		{
			return new TEditor();
		}

		// This function ensures that T is of type Object by causing a compiler error if it isn't
		void CompilerTest();
	};
}