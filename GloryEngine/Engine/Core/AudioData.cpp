#include "AudioData.h"
#include "BinaryStream.h"

namespace Glory
{
	AudioData::AudioData()
	{
		APPEND_TYPE(AudioData);
	}

	AudioData::AudioData(std::vector<char>&& data) : m_Data(std::move(data))
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

	void AudioData::Serialize(BinaryStream& container) const
	{
		container.Write(m_Data.size()).Write(m_Data.data(), m_Data.size());
	}

	void AudioData::Deserialize(BinaryStream& container)
	{
		size_t size;
		container.Read(size);
		m_Data.resize(size);
		container.Read(m_Data.data(), m_Data.size());
	}
}
