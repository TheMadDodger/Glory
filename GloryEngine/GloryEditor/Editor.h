//#pragma once
//
//namespace Spartan::Editor
//{
//	class Editor
//	{
//	public:
//		template<class T>
//		static void RegisterEditor()
//		{
//			Editor* pEditor = new T();
//			m_pRegisteredEditors.push_back(pEditor);
//		}
//
//		virtual const std::type_info& GetEditedType() = 0;
//		virtual void OnGUI() = 0;
//
//		static Editor* CreateEditor(SEObject* pObject);
//
//		static size_t GetID(Editor* pEditor);
//
//	protected:
//		SEObject* m_pTarget;
//
//	protected:
//		Editor();
//		virtual ~Editor();
//
//	private:
//		friend class EditorApp;
//		friend class InspectorWindow;
//		static void Cleanup();
//		virtual Editor* Create() = 0;
//
//	private:
//		static std::vector<Editor*> m_pRegisteredEditors;
//		static std::vector<Editor*> m_pActiveEditors;
//	};
//
//	template<class TEditor, class TObject>
//	class EditorTemplate : public Editor
//	{
//	public:
//		virtual const std::type_info& GetEditedType() override
//		{
//			return typeid(TObject);
//		}
//
//	private:
//		virtual Editor* Create() override
//		{
//			return new TEditor();
//		}
//
//		// This function ensures that T is of type SEObject by causing a compiler error if it isn't
//		void CompilerTest();
//	};
//}