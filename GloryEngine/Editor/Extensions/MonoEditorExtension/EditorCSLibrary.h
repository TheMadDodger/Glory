#pragma once

#include <UUID.h>

#include <GloryEditor.h>

#include <mono/jit/jit.h>

#include <string>

namespace Glory
{
	class GloryMonoScipting;
	class SceneObject;
	class MonoScript;
}

namespace Glory::Editor
{
	GLORY_EDITOR_API void SetMonoScriptModule(GloryMonoScipting* pModule);
	GLORY_EDITOR_API bool InvokeDrawInspector(MonoScript* pScript, UUID sceneID, UUID objectID);
	GLORY_EDITOR_API bool InvokeMethod(const std::string& ns, const std::string& className, const std::string& method, void** args);
}
