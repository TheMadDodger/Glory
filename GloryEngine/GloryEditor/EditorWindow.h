#pragma once
#include "EditorWindow.h"
#include <vector>
#include <list>
#include <string>
#include <algorithm>
#include <imgui.h>
#include <typeinfo>

namespace Glory::Editor
{
	class EditorWindow
	{
	public:
		EditorWindow(const std::string& windowName, float windowWidth, float windowHeight);
		virtual ~EditorWindow();

		template<typename T>
		static T* GetWindow(bool alwaysOpenNew = false)
		{
			if (!alwaysOpenNew)
			{
				for (size_t i = 0; i < m_pActiveEditorWindows.size(); ++i)
				{
					if (m_pActiveEditorWindows[i]->GetType() == typeid(T))
						return (T*)m_pActiveEditorWindows[i];
				}
			}

			auto pWindow = new T();
			m_pActiveEditorWindows.push_back(pWindow);
			
			if (m_IDs.size() > 0)
			{
				pWindow->m_WindowID = m_IDs.front();
				m_IDs.pop_front();
			}
			else pWindow->m_WindowID = m_pActiveEditorWindows.size();

			pWindow->OnOpen();
			return pWindow;
		}

		static void UpdateWindows();

		virtual const std::type_info& GetType() = 0;

		void Close();

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