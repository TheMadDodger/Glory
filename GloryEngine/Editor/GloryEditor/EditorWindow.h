#pragma once
#include "GloryEditor.h"
#include "DND.h"

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
		GLORY_EDITOR_API EditorWindow(const std::string& windowName, float windowWidth, float windowHeight);
		GLORY_EDITOR_API virtual ~EditorWindow();

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

		GLORY_EDITOR_API static EditorWindow* FindEditorWindow(const std::type_info& type);
		GLORY_EDITOR_API static void OpenEditorWindow(EditorWindow* pWindow);

		GLORY_EDITOR_API static void UpdateWindows();

		virtual const std::type_info& GetType() = 0;

		GLORY_EDITOR_API void Close();

		virtual void OnOpen() {}
		virtual void OnClose() {}
		virtual void HandleDragAndDropPayload(uint32_t, const ImGuiPayload*) {}

		GLORY_EDITOR_API const bool IsFocused() const;

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
		friend class SceneEditingMainWindow;
		const std::string m_WindowName;
		ImVec2 m_WindowDimensions;
		static std::vector<EditorWindow*> m_pActiveEditorWindows;
		static std::vector<EditorWindow*> m_pClosingEditorWindows;
		static std::list<size_t> m_IDs;
		bool m_IsOpen;
		bool m_Resizeable;
		int m_WindowFlags;
		size_t m_WindowID;
		bool m_IsFocused;
		DND m_DragAndDrop;
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