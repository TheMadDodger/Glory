#pragma once
#include <vector>
#include "BaseEditorExtension.h"
#include "EditorWindowImpl.h"
#include "EditorRenderImpl.h"
#include "GloryEditor.h"

namespace Glory
{
	class Engine;

	struct EditorCreateInfo
	{
		GLORY_EDITOR_API EditorCreateInfo();

		uint32_t ExtensionsCount;
		Editor::BaseEditorExtension** pExtensions;

		Editor::EditorWindowImpl* pWindowImpl;
		Editor::EditorRenderImpl* pRenderImpl;

		Engine* pEngine;
	};
}