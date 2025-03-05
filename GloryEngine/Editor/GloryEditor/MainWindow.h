#pragma once
#include <GloryEditor.h>

#include <string_view>
#include <typeindex>
#include <vector>
#include <list>

namespace Glory::Editor
{
	class Toolbar;
	class EditorWindow;

	class MainWindow
	{
	public:
		GLORY_EDITOR_API MainWindow();
		GLORY_EDITOR_API virtual ~MainWindow();

		void DrawGui(float height);

		virtual std::type_index Type() = 0;
		virtual std::string_view Name() = 0;
		virtual void OnGui(float height) = 0;
		virtual void Initialize() = 0;

		void RenderWindows();

		template<typename T>
		T* GetWindow(bool alwaysOpenNew = false)
		{
			const std::type_info& type = typeid(T);
			EditorWindow* pWindow = nullptr;
			if (!alwaysOpenNew)
			{
				pWindow = FindEditorWindow(type);
				if (pWindow) return (T*)pWindow;
			}

			pWindow = new T();
			pWindow->m_pOwner = this;
			OpenEditorWindow(pWindow);
			return (T*)pWindow;
		}

		GLORY_EDITOR_API EditorWindow* FindEditorWindow(const std::type_info& type);
		GLORY_EDITOR_API void OpenEditorWindow(EditorWindow* pWindow);

		GLORY_EDITOR_API void UpdateWindows();

	protected:
		GLORY_EDITOR_API void Dockspace(float height);

	private:
		friend class MainEditor;
		friend class EditorWindow;
		std::vector<EditorWindow*> m_pActiveEditorWindows;
		std::vector<EditorWindow*> m_pClosingEditorWindows;
		std::list<size_t> m_IDs;
	};

	template<typename T>
	class MainWindowTemplate : public MainWindow
	{
	private:
		virtual std::type_index Type() { return typeid(T); }
	};

	class SceneEditingMainWindow : public MainWindowTemplate<SceneEditingMainWindow>
	{
	public:
		SceneEditingMainWindow();
		virtual ~SceneEditingMainWindow();

		static const float TOOLBAR_SIZE;

		virtual std::string_view Name() override;
		virtual void OnGui(float height) override;
		virtual void Initialize() override;

	private:
		Toolbar* m_pToolbar;
	};
}
