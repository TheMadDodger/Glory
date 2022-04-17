#include <SDLWindowModule.h>
#include <Game.h>
#include <Engine.h>
#include "EditorWindowImpl.h"

namespace Glory::Editor
{
	EditorWindowImpl::EditorWindowImpl() {}

	EditorWindowImpl::~EditorWindowImpl() {}

	void EditorWindowImpl::Initialize()
	{
		WindowModule* pWindowModule = Game::GetGame().GetEngine()->GetWindowModule();
		m_pMainWindow = pWindowModule->GetMainWindow();
	}

	Window* EditorWindowImpl::GetMainWindow()
	{
		return m_pMainWindow;
	}
}