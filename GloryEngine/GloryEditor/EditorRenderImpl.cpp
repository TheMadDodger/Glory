#include "EditorRenderImpl.h"

namespace Glory::Editor
{
	GLORY_EDITOR_API EditorRenderImpl::EditorRenderImpl() {}

	GLORY_EDITOR_API EditorRenderImpl::~EditorRenderImpl() {}

	void EditorRenderImpl::Initialize()
	{
		Setup();
	}

	void EditorRenderImpl::Render()
	{
	}

	void EditorRenderImpl::Destroy()
	{
		ImGui::DestroyContext();
	}
}