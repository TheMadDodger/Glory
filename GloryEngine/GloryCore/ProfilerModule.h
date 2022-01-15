#pragma once
#include "Module.h"
#include <string>
#include <unordered_map>
#include <thread>
#include <mutex>
#include "ProfilerThreadSample.h"

namespace Glory
{
	class ProfilerModule : public Module
	{
	public:
		ProfilerModule();
		virtual ~ProfilerModule();

		void BeginThread(const std::string& name);
		void EndThread();
		void BeginSample(const std::string& name);
		void EndSample();

		void EnableSampleCollecting(bool enabled);

		void StartRecording();
		void EndRecording();

	private:
		virtual const std::type_info& GetModuleType() override;

		virtual void Initialize() override;
		virtual void Cleanup() override;

		virtual void OnGameThreadFrameStart() override;
		virtual void OnGameThreadFrameEnd() override;
		virtual void OnGraphicsThreadFrameStart() override;
		virtual void OnGraphicsThreadFrameEnd() override;

		void StoreSampleRecord(const ProfilerThreadSample& sample);

	private:
		std::unordered_map<std::string, ProfilerThreadSample> m_CurrentThreadSamples;
		std::unordered_map<std::thread::id, std::string> m_ThreadIDToProfile;

		std::mutex m_RecordMutex;

		bool m_SampleCollectingEnabled;
		bool m_IsRecording;

		static const size_t MAX_SAMPLE_RECORDS = 100;
		size_t m_CurrentSampleRecordSize;
		int m_CurrentSampleWrite;
		ProfilerThreadSample m_SampleRecords[MAX_SAMPLE_RECORDS];
	};
}
