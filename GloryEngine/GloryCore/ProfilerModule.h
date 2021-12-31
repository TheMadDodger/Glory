#pragma once
#include "Module.h"
#include <string>
#include <unordered_map>
#include <thread>
#include <mutex>
#include "ProfilerThreadSample.h"
#include <functional>

namespace Glory
{
	class ProfilerModule : public Module
	{
	public:
		ProfilerModule();
		virtual ~ProfilerModule();

		void RegisterRecordCallback(std::function<void(const ProfilerThreadSample&)> callback);

		void BeginThread(const std::string& name);
		void EndThread();
		void BeginSample(const std::string& name);
		void EndSample();

		void EnableSampleCollecting(bool enabled);

	private:
		virtual const std::type_info& GetModuleType() override;

		virtual void Initialize() override;
		virtual void Cleanup() override;

		virtual void OnGameThreadFrameStart() override;
		virtual void OnGameThreadFrameEnd() override;
		virtual void OnGraphicsThreadFrameStart() override;
		virtual void OnGraphicsThreadFrameEnd() override;

	private:
		std::unordered_map<std::string, ProfilerThreadSample> m_CurrentThreadSamples;
		std::unordered_map<std::thread::id, std::string> m_ThreadIDToProfile;
		std::function<void(const ProfilerThreadSample&)> m_RecordCallback;
		bool m_SampleCollectingEnabled;
	};
}
