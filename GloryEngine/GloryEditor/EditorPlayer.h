#pragma once
#include "IEditorLoopHandler.h"
#include <IModuleLoopHandler.h>
#include <GScene.h>
#include <vector>
#include "GloryEditor.h"

namespace Glory::Editor
{
	class EditorPlayer : public IModuleLoopHandler
	{
	public:
		static GLORY_EDITOR_API void RegisterLoopHandler(IEditorLoopHandler* pEditorSceneLoopHandler);

	private:
		void Start();
		void Stop();
		void TogglePauze();
		void TickFrame();

		virtual bool HandleModuleLoop(Module* pModule) override;

	private:
		friend class EditorApplication;
		EditorPlayer();
		virtual ~EditorPlayer();

		std::string m_SerializedScenes;
		size_t m_UndoHistoryIndex;
		UUID m_SelectedObjectBeforeStart;
		bool m_IsPaused;
		bool m_FrameRequested;
		static std::vector<IEditorLoopHandler*> m_pSceneLoopHandlers;
	};
}
