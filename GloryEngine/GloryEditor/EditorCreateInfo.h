#pragma once
#include <vector>
#include <Game.h>
#include <BaseEditorExtension.h>

namespace Glory
{
	struct EditorCreateInfo
	{
		EditorCreateInfo();

		uint32_t ExtensionsCount;
		Editor::BaseEditorExtension** pExtensions;
	};
}