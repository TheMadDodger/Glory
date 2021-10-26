#pragma once
#include "EditorPlatform.h"
#include "MainEditor.h"
#include <Game.h>
#include <EditorAssetLoader.h>

namespace Glory::Editor
{
	class EditorApplication
	{
	public:
		EditorApplication();
		virtual ~EditorApplication();

		template<class Window, class Renderer>
		void Initialize(Game& game)
		{
			auto window = (EditorWindowImpl*)(new Window());
			auto renderer = (EditorRenderImpl*)(new Renderer());
			m_pPlatform = new EditorPlatform(window, renderer);
			window->m_pEditorPlatform = m_pPlatform;
			renderer->m_pEditorPlatform = m_pPlatform;
			m_pPlatform->Initialize(game);
			m_pMainEditor = new MainEditor();
			m_pMainEditor->Initialize();

			m_pEditorInstance = this;
		}

		void Destroy();
		void Run();

		EditorPlatform* GetEditorPlatform();

		static EditorApplication* GetInstance();

	private:
		void RenderEditor();

	private:
		EditorPlatform* m_pPlatform;
		MainEditor* m_pMainEditor;
		static EditorApplication* m_pEditorInstance;
	};
}
