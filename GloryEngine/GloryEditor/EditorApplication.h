#pragma once
#include "EditorPlatform.h"
#include "MainEditor.h"
#include "ProjectSpace.h"
#include "EditorShaderProcessor.h"
#include "EditorAssetLoader.h"
#include "EditorCreateInfo.h"
#include "EditorPlayer.h"
#include <Versioning.h>
#include <Game.h>

namespace Glory::Editor
{
	enum class EditorMode
	{
		M_Edit,
		M_EnteringPlay,
		M_Play,
		M_ExitingPlay,
	};

	class EditorApplication
	{
	public:
		GLORY_EDITOR_API EditorApplication(const EditorCreateInfo& createInfo);
		virtual GLORY_EDITOR_API ~EditorApplication();

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

		GLORY_EDITOR_API void Initialize(Game& game);


		GLORY_EDITOR_API void Destroy();
		GLORY_EDITOR_API void Run(Game& game);

		GLORY_EDITOR_API void SetWindowImpl(EditorWindowImpl* pWindowImpl);
		GLORY_EDITOR_API void SetRendererImpl(EditorRenderImpl* pRendererImpl);

		GLORY_EDITOR_API EditorPlatform* GetEditorPlatform();
		GLORY_EDITOR_API MainEditor* GetMainEditor();

		static GLORY_EDITOR_API EditorApplication* GetInstance();
		static GLORY_EDITOR_API const EditorMode& CurrentMode();
		static GLORY_EDITOR_API void StartPlay();
		static GLORY_EDITOR_API void StopPlay();
		static GLORY_EDITOR_API void TogglePause();
		static GLORY_EDITOR_API void TickFrame();
		static GLORY_EDITOR_API bool IsPaused();

		static const Glory::Version Version;

	private:
		void RenderEditor();
		void InitializePlatform(Game& game);
		void InitializeExtensions();

		static std::string AssetPathOverrider();

	private:
		EditorWindowImpl* m_pTempWindowImpl;
		EditorRenderImpl* m_pTempRenderImpl;

		EditorPlatform* m_pPlatform;
		MainEditor* m_pMainEditor;
		EditorPlayer* m_pPlayer;
		std::vector<BaseEditorExtension*> m_pExtensions;
		EditorShaderProcessor* m_pShaderProcessor;
		static EditorApplication* m_pEditorInstance;
		static EditorMode m_Mode;
	};
}
