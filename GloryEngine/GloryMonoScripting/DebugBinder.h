#pragma once
#include "ScriptBinding.h"
#include <mono/metadata/object.h>
#include <Debug.h>

namespace Glory
{
	class DebugBinder
	{
	private:
		static void Log(MonoString* message, Debug::LogLevel logLevel, MonoBoolean bIncludeTimeStamp);
		static void LogInfo(MonoString* message, MonoBoolean bIncludeTimeStamp);
		static void LogNotice(MonoString* message, MonoBoolean bIncludeTimeStamp);
		static void LogWarning(MonoString* message, MonoBoolean bIncludeTimeStamp);
		static void LogError(MonoString* message, MonoBoolean bIncludeTimeStamp);
		static void LogFatalError(MonoString* message, MonoBoolean bIncludeTimeStamp);
		static void LogOnce(MonoString* key, MonoString* message, Debug::LogLevel logLevel, MonoBoolean bIncludeTimeStamp = true);
		static void CreateBindings(std::vector<ScriptBinding>& bindings);
	};
}
