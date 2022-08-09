#include "EditorCreateInfo.h"

namespace Glory
{
	GLORY_EDITOR_API EditorCreateInfo::EditorCreateInfo()
		: ExtensionsCount(0), pExtensions(nullptr), pWindowImpl(nullptr), pRenderImpl(nullptr)
	{
	}
}