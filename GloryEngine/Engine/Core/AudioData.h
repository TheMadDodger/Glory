#pragma once
#include "Resource.h"

namespace Glory
{
	class AudioData : public Resource
	{
	public:
		AudioData();
		AudioData(std::vector<char>&& data);
		virtual ~AudioData() = default;

		const char* Data() const;
		const size_t Size() const;

	private:
		std::vector<char> m_Data;
	};
}
