#pragma once
#include "ProjectPopup.h"
#include <EditorAssetLoader.h>
#include <GameThread.h>

namespace Glory::Editor
{
	class MainEditor
	{
	public:
		MainEditor();
		virtual ~MainEditor();

		void Initialize();
		void Destroy();
		void PaintEditor();

		EditorAssetLoader* GetAssetLoader();

	private:
		void CreateDefaultMainMenuBar();
		void SetDarkThemeColors();

		void Tick();
		void Paint();

	private:
		friend class Glory::GameThread;
		EditorAssetLoader* m_pAssetLoader;
		ProjectPopup* m_pProjectPopup;
	};
}
