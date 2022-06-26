#pragma once
#include <vector>
#include <Game.h>
#include <BaseEditorExtension.h>
#include <EditorWindowImpl.h>
#include <EditorRenderImpl.h>

namespace Glory
{
	struct EditorCreateInfo
	{
		EditorCreateInfo();

		uint32_t ExtensionsCount;
		Editor::BaseEditorExtension** pExtensions;

		Editor::EditorWindowImpl* pWindowImpl;
		Editor::EditorRenderImpl* pRenderImpl;
	};
}