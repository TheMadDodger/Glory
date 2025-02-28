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
	class MainWindow;

	class EditorWindow
	{
	public:
		GLORY_EDITOR_API EditorWindow(const std::string& windowName, float windowWidth, float windowHeight);
		GLORY_EDITOR_API virtual ~EditorWindow();

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

	protected:
		friend class MainEditor;
		friend class MainWindow;
		MainWindow* m_pOwner;

		const std::string m_WindowName;
		ImVec2 m_WindowDimensions;
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