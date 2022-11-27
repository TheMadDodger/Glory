#include <Game.h>
#include <Engine.h>
#include "EditorWindowImpl.h"

namespace Glory::Editor
{
	GLORY_EDITOR_API EditorWindowImpl::EditorWindowImpl() {}

	GLORY_EDITOR_API EditorWindowImpl::~EditorWindowImpl() {}

	GLORY_EDITOR_API void EditorWindowImpl::Initialize()
	{
		WindowModule* pWindowModule = Game::GetGame().GetEngine()->GetWindowModule();
		m_pMainWindow = pWindowModule->GetMainWindow();
	}

	GLORY_EDITOR_API Window* EditorWindowImpl::GetMainWindow()
	{
		return m_pMainWindow;
	}
}