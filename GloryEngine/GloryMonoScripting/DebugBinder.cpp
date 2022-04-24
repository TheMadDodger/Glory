#include "DebugBinder.h"

namespace Glory
{
	void DebugBinder::Log(MonoString* message, Debug::LogLevel logLevel, MonoBoolean bIncludeTimeStamp)
	{
		Debug::Log(mono_string_to_utf8(message), logLevel, bIncludeTimeStamp);
	}

	void DebugBinder::LogInfo(MonoString* message, MonoBoolean bIncludeTimeStamp)
	{
		Debug::LogInfo(mono_string_to_utf8(message), bIncludeTimeStamp);
	}

	void DebugBinder::LogNotice(MonoString* message, MonoBoolean bIncludeTimeStamp)
	{
		Debug::LogNotice(mono_string_to_utf8(message), bIncludeTimeStamp);
	}

	void DebugBinder::LogWarning(MonoString* message, MonoBoolean bIncludeTimeStamp)
	{
		Debug::LogWarning(mono_string_to_utf8(message), bIncludeTimeStamp);
	}

	void DebugBinder::LogError(MonoString* message, MonoBoolean bIncludeTimeStamp)
	{
		Debug::LogError(mono_string_to_utf8(message), bIncludeTimeStamp);
	}

	void DebugBinder::LogFatalError(MonoString* message, MonoBoolean bIncludeTimeStamp)
	{
		Debug::LogFatalError(mono_string_to_utf8(message), bIncludeTimeStamp);
	}

	void DebugBinder::LogOnce(MonoString* key, MonoString* message, Debug::LogLevel logLevel, MonoBoolean bIncludeTimeStamp)
	{
		Debug::LogOnce(mono_string_to_utf8(key), mono_string_to_utf8(message), logLevel, bIncludeTimeStamp);
	}

	void DebugBinder::CreateBindings(std::vector<ScriptBinding>& bindings)
	{
		bindings.push_back(ScriptBinding("C#", "GloryEngine.Debug::Log(string,GloryEngine.LogLevel,bool)", &DebugBinder::Log));
		bindings.push_back(ScriptBinding("C#", "GloryEngine.Debug::LogInfo(string,bool)", &DebugBinder::LogInfo));
		bindings.push_back(ScriptBinding("C#", "GloryEngine.Debug::LogNotice(string,bool)", &DebugBinder::LogNotice));
		bindings.push_back(ScriptBinding("C#", "GloryEngine.Debug::LogWarning(string,bool)", &DebugBinder::LogWarning));
		bindings.push_back(ScriptBinding("C#", "GloryEngine.Debug::LogError(string,bool)", &DebugBinder::LogError));
		bindings.push_back(ScriptBinding("C#", "GloryEngine.Debug::LogFatalError(string,bool)", &DebugBinder::LogFatalError));
		bindings.push_back(ScriptBinding("C#", "GloryEngine.Debug::LogOnce(string,string,GloryEngine.LogLevel,bool)", &DebugBinder::LogOnce));
	}
}