#pragma once
#include <vector>
#include <list>
#include <string>
#include <algorithm>
#include <imgui.h>
#include <typeinfo>
#include "GloryEditor.h"

namespace Glory::Editor
{
	class EditorWindow
	{
	public:
		GLORY_EDITOR_API EditorWindow(const std::string& windowName, float windowWidth, float windowHeight);
		virtual GLORY_EDITOR_API ~EditorWindow();

		template<typename T>
		static T* GetWindow(bool alwaysOpenNew = false)
		{
			const std::type_info& type = typeid(T);
			EditorWindow* pWindow = nullptr;
			if (!alwaysOpenNew)
			{
				pWindow = FindEditorWindow(type);
				if (pWindow) return (T*)pWindow;
			}

			pWindow = new T();
			OpenEditorWindow(pWindow);
			return (T*)pWindow;
		}

		static GLORY_EDITOR_API EditorWindow* FindEditorWindow(const std::type_info& type);
		static GLORY_EDITOR_API void OpenEditorWindow(EditorWindow* pWindow);

		static GLORY_EDITOR_API void UpdateWindows();

		virtual const std::type_info& GetType() = 0;

		GLORY_EDITOR_API void Close();

		virtual void OnOpen() {}
		virtual void OnClose() {}

	protected:
		virtual void OnGUI() = 0;

		virtual void Update() {}
		virtual void Draw() {}

	private:
		void RenderGUI();

		static void RenderWindows();
		static void Cleanup();

	protected:
		friend class MainEditor;
		const std::string m_WindowName;
		ImVec2 m_WindowDimensions;
		static std::vector<EditorWindow*> m_pActiveEditorWindows;
		static std::vector<EditorWindow*> m_pClosingEditorWindows;
		static std::list<size_t> m_IDs;
		bool m_IsOpen;
		bool m_Resizeable;
		int m_WindowFlags;
		size_t m_WindowID;
	};

	template<typename T>
	class EditorWindowTemplate : public EditorWindow
	{
	public:
		EditorWindowTemplate(const std::string& windowName, float windowWidth, float windowHeight) : EditorWindow(windowName, windowWidth, windowHeight) {}
		virtual ~EditorWindowTemplate() {}

	private:
		virtual const std::type_info& GetType() override
		{
			return typeid(T);
		}
	};
}