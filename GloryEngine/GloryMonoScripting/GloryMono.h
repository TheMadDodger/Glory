#pragma once
#pragma comment (lib,"mono-2.0-sgen.lib")
#pragma comment (lib,"MonoPosixHelper.lib")

#include <Glory.h>
#include <Module.h>
#include <GloryContext.h>
#include <ScriptingModule.h>
#include <ScriptExtensions.h>
#include <IScriptExtender.h>
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/debug-helpers.h>

extern "C" GLORY_API Glory::Module * OnLoadModule(Glory::GloryContext * pContext);
