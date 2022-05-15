#include "DebugBinder.h"
#include "GloryMonoScipting.h"

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

	void DebugBinder::CreateBindings(std::vector<InternalCall>& internalCalls)
	{
		BIND("GloryEngine.Debug::Log(string,GloryEngine.LogLevel,bool)", DebugBinder::Log);
		BIND("GloryEngine.Debug::LogInfo(string,bool)", DebugBinder::LogInfo);
		BIND("GloryEngine.Debug::LogNotice(string,bool)", DebugBinder::LogNotice);
		BIND("GloryEngine.Debug::LogWarning(string,bool)", DebugBinder::LogWarning);
		BIND("GloryEngine.Debug::LogError(string,bool)", DebugBinder::LogError);
		BIND("GloryEngine.Debug::LogFatalError(string,bool)", DebugBinder::LogFatalError);
		BIND("GloryEngine.Debug::LogOnce(string,string,GloryEngine.LogLevel,bool)", DebugBinder::LogOnce);
	}
}