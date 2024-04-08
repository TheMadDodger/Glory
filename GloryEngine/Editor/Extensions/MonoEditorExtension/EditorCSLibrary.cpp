#include "EditorCSLibrary.h"

#include <GloryMonoScipting.h>
#include <MonoScript.h>
#include <MonoManager.h>
#include <MonoScriptObjectManager.h>
#include <AssemblyDomain.h>
#include <Assembly.h>

namespace Glory::Editor
{
	GloryMonoScipting* Scripting;

	void SetMonoScriptModule(GloryMonoScipting* pModule)
	{
		Scripting = pModule;
	}

	bool InvokeDrawInspector(MonoScript* pScript, UUID sceneID, UUID objectID)
	{
		Assembly* pMainAssembly = Scripting->GetMonoManager()->ActiveDomain()->GetMainAssembly();

		MonoObject* pMonoObject = pScript->GetScriptObject(pMainAssembly, objectID, sceneID);
		if (!pMonoObject) return false;

		void* args[] = {
			pMonoObject
		};

		return InvokeMethod("GloryEngine.Editor", "Editor", ".::DrawInspectorGUI", args);
	}

	bool InvokeMethod(const std::string& ns, const std::string& className, const std::string& method, void** args)
	{
		Assembly* pAssembly = Scripting->GetMonoManager()->ActiveDomain()->GetAssembly("GloryEngine.Editor.dll");
		AssemblyClass* pClass = pAssembly->GetClass(ns, className);
		MonoMethod* pMethod = pClass->GetMethod(method);
		MonoObject* result = Scripting->GetMonoManager()->ActiveDomain()->InvokeMethod(pMethod, nullptr, args);
		return result ? *(bool*)result : false;
	}
}
