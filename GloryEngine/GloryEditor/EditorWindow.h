#pragma once

namespace Glory::Editor
{
	class EditorWindow
	{
	public:
		EditorWindow(const std::string& windowName, float windowWidth, float windowHeight);
		virtual ~EditorWindow();

		template<typename T>
		static EditorWindow* GetWindow(bool alwaysOpenNew = false)
		{
			if (!alwaysOpenNew)
			{
				for (size_t i = 0; i < m_pActiveEditorWindows.size(); ++i)
				{
					if (m_pActiveEditorWindows[i]->GetType() == typeid(T))
						return m_pActiveEditorWindows[i];
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

			return pWindow;
		}

		virtual const std::type_info& GetType() = 0;

		void Close();

	protected:
		virtual void OnPaint() {};
		virtual void OnGUI() = 0;

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