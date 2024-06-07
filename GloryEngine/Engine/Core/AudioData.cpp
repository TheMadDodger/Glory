#include "AudioData.h"

namespace Glory
{
	AudioData::AudioData()
	{
		APPEND_TYPE(AudioData);
	}
	
	AudioData::AudioData(std::vector<char>&& data): m_Data(std::move(data))
	{
		APPEND_TYPE(AudioData);
	}

	const char* AudioData::Data() const
	{
		return m_Data.data();
	}

	const size_t AudioData::Size() const
	{
		return m_Data.size();
	}
}
