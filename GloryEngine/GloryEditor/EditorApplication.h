#pragma once
#include "EditorPlatform.h"
#include "MainEditor.h"
#include "ProjectSpace.h"
#include "EditorShaderProcessor.h"
#include <Game.h>
#include <EditorAssetLoader.h>
#include <EditorCreateInfo.h>

namespace Glory::Editor
{
	class EditorApplication
	{
	public:
		EditorApplication(const EditorCreateInfo& createInfo);
		virtual ~EditorApplication();

		template<class Window, class Renderer>
		void Initialize(Game& game)
		{
			game.OverrideAssetPathFunc(EditorApplication::AssetPathOverrider);

			auto window = (EditorWindowImpl*)(new Window());
			auto renderer = (EditorRenderImpl*)(new Renderer());
			m_pPlatform = new EditorPlatform(window, renderer);
			window->m_pEditorPlatform = m_pPlatform;
			renderer->m_pEditorPlatform = m_pPlatform;
			
			InitializePlatform(game);
		}

		void Initialize(Game& game);

		void InitializeExtensions();

		void Destroy();
		void Run(Game& game);

		void SetWindowImpl(EditorWindowImpl* pWindowImpl);
		void SetRendererImpl(EditorRenderImpl* pRendererImpl);

		EditorPlatform* GetEditorPlatform();
		MainEditor* GetMainEditor();

		static EditorApplication* GetInstance();

	private:
		void RenderEditor();
		void InitializePlatform(Game& game);

		static std::string AssetPathOverrider();

	private:
		EditorWindowImpl* m_pTempWindowImpl;
		EditorRenderImpl* m_pTempRenderImpl;

		EditorPlatform* m_pPlatform;
		MainEditor* m_pMainEditor;
		std::vector<BaseEditorExtension*> m_pExtensions;
		EditorShaderProcessor* m_pShaderProcessor;
		static EditorApplication* m_pEditorInstance;
	};
}
