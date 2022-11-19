#include "CoreCSAPI.h"
#include "GloryMonoScipting.h"
#include <GameTime.h>
#include <LayerManager.h>
#include "MonoManager.h"
#include <EngineProfiler.h>

namespace Glory
{
#pragma region Debug
	void Log(MonoString* message, Debug::LogLevel logLevel, MonoBoolean bIncludeTimeStamp)
	{
		Debug::Log(mono_string_to_utf8(message), logLevel, bIncludeTimeStamp);
	}

	void LogInfo(MonoString* message, MonoBoolean bIncludeTimeStamp)
	{
		Debug::LogInfo(mono_string_to_utf8(message), bIncludeTimeStamp);
	}

	void LogNotice(MonoString* message, MonoBoolean bIncludeTimeStamp)
	{
		Debug::LogNotice(mono_string_to_utf8(message), bIncludeTimeStamp);
	}

	void LogWarning(MonoString* message, MonoBoolean bIncludeTimeStamp)
	{
		Debug::LogWarning(mono_string_to_utf8(message), bIncludeTimeStamp);
	}

	void LogError(MonoString* message, MonoBoolean bIncludeTimeStamp)
	{
		Debug::LogError(mono_string_to_utf8(message), bIncludeTimeStamp);
	}

	void LogFatalError(MonoString* message, MonoBoolean bIncludeTimeStamp)
	{
		Debug::LogFatalError(mono_string_to_utf8(message), bIncludeTimeStamp);
	}

	void LogOnce(MonoString* key, MonoString* message, Debug::LogLevel logLevel, MonoBoolean bIncludeTimeStamp)
	{
		Debug::LogOnce(mono_string_to_utf8(key), mono_string_to_utf8(message), logLevel, bIncludeTimeStamp);
	}
#pragma endregion

#pragma region Game Time
	float Time_GetDeltaTime()
	{
		return Time::GetDeltaTime<float, std::ratio<1, 1>>();
	}

	float Time_GetGameDeltaTime()
	{
		return Time::GetGameDeltaTime<float, std::ratio<1, 1>>();
	}

	float Time_GetGraphicsDeltaTime()
	{
		return Time::GetGraphicsDeltaTime<float, std::ratio<1, 1>>();
	}

	float Time_GetUnscaledDeltaTime()
	{
		return Time::GetUnscaledDeltaTime<float, std::ratio<1, 1>>();
	}

	float Time_GetUnscaledGameDeltaTime()
	{
		return Time::GetUnscaledGameDeltaTime<float, std::ratio<1, 1>>();
	}

	float Time_GetUnscaledGraphicsDeltaTime()
	{
		return Time::GetUnscaledGraphicsDeltaTime<float, std::ratio<1, 1>>();
	}

	float Time_GetCurrentTime()
	{
		return Time::GetTime();
	}

	float Time_GetUnscaledTime()
	{
		return Time::GetUnscaledTime();
	}

	float Time_GetFrameRate()
	{
		return Time::GetFrameRate();
	}

	int Time_GetTotalFrames()
	{
		return Time::GetTotalFrames();
	}

	int Time_GetTotalGameFrames()
	{
		return Time::GetTotalGameFrames();
	}

	float Time_GetTimeScale()
	{
		return Time::GetTimeScale();
	}

	void Time_SetTimeScale(float scale)
	{
		return Time::SetTimeScale(scale);
	}
#pragma endregion

#pragma region Layer Management
	void LayerManager_AddLayer(MonoString* name)
	{
		const std::string nameStr = mono_string_to_utf8(name);
		LayerManager::AddLayer(nameStr);
	}

	void LayerManager_Load()
	{
		LayerManager::Load();
	}

	void LayerManager_Save()
	{
		LayerManager::Save();
	}

	LayerWrapper LayerManager_GetLayerByName(MonoString* name)
	{
		const std::string nameStr = mono_string_to_utf8(name);
		const Layer* layer = LayerManager::GetLayerByName(nameStr);
		return LayerWrapper(layer);
	}

	MonoString* LayerManager_LayerMaskToString(LayerMask* layerMask)
	{
		const std::string str = LayerManager::LayerMaskToString(*layerMask);
		return mono_string_new(MonoManager::GetDomain(), str.c_str());
	}

	int LayerManager_GetLayerIndex(Layer* pLayer)
	{
		return LayerManager::GetLayerIndex(pLayer);
	}

	LayerWrapper LayerManager_GetLayerAtIndex(int index)
	{
		const Layer* layer = LayerManager::GetLayerAtIndex(index);
		return LayerWrapper(layer);
	}

#pragma endregion

#pragma region Profiler

	void Profiler_BeginSample(MonoString* name)
	{
		const std::string nameStr = mono_string_to_utf8(name);
		Profiler::BeginSample(nameStr);
	}

	void Profiler_EndSample()
	{
		Profiler::EndSample();
	}

#pragma endregion



#pragma region Binding

	void CoreCSAPI::AddInternalCalls(std::vector<InternalCall>& internalCalls)
	{
		BIND("GloryEngine.Debug::Log(string,GloryEngine.LogLevel,bool)", Log);
		BIND("GloryEngine.Debug::LogInfo(string,bool)", LogInfo);
		BIND("GloryEngine.Debug::LogNotice(string,bool)", LogNotice);
		BIND("GloryEngine.Debug::LogWarning(string,bool)", LogWarning);
		BIND("GloryEngine.Debug::LogError(string,bool)", LogError);
		BIND("GloryEngine.Debug::LogFatalError(string,bool)", LogFatalError);
		BIND("GloryEngine.Debug::LogOnce(string,string,GloryEngine.LogLevel,bool)", LogOnce);

		BIND("GloryEngine.Time::Time_GetDeltaTime", Time_GetDeltaTime);
		BIND("GloryEngine.Time::Time_GetGameDeltaTime", Time_GetGameDeltaTime);
		BIND("GloryEngine.Time::Time_GetGraphicsDeltaTime", Time_GetGraphicsDeltaTime);
		BIND("GloryEngine.Time::Time_GetUnscaledDeltaTime", Time_GetUnscaledDeltaTime);
		BIND("GloryEngine.Time::Time_GetUnscaledGameDeltaTime", Time_GetUnscaledGameDeltaTime);
		BIND("GloryEngine.Time::Time_GetUnscaledGraphicsDeltaTime", Time_GetUnscaledGraphicsDeltaTime);
		BIND("GloryEngine.Time::Time_GetCurrentTime", Time_GetCurrentTime);
		BIND("GloryEngine.Time::Time_GetUnscaledTime", Time_GetUnscaledTime);
		BIND("GloryEngine.Time::Time_GetFrameRate", Time_GetFrameRate);
		BIND("GloryEngine.Time::Time_GetTotalFrames", Time_GetTotalFrames);
		BIND("GloryEngine.Time::Time_GetTotalGameFrames", Time_GetTotalGameFrames);
		BIND("GloryEngine.Time::Time_GetTimeScale", Time_GetTimeScale);
		BIND("GloryEngine.Time::Time_SetTimeScale", Time_SetTimeScale);

		BIND("GloryEngine.LayerManager::AddLayer", LayerManager_AddLayer);
		BIND("GloryEngine.LayerManager::Load", LayerManager_Load);
		BIND("GloryEngine.LayerManager::Save", LayerManager_Save);
		BIND("GloryEngine.LayerManager::GetLayerByName", LayerManager_GetLayerByName);
		BIND("GloryEngine.LayerManager::LayerMaskToString", LayerManager_LayerMaskToString);
		BIND("GloryEngine.LayerManager::GetLayerIndex", LayerManager_GetLayerIndex);
		BIND("GloryEngine.LayerManager::GetLayerAtIndex", LayerManager_GetLayerAtIndex);

		BIND("GloryEngine.Profiler::BeginSample", Profiler_BeginSample);
		BIND("GloryEngine.Profiler::EndSample", Profiler_EndSample);
	}

	CoreCSAPI::CoreCSAPI() {}
	CoreCSAPI::~CoreCSAPI() {}

#pragma endregion
}