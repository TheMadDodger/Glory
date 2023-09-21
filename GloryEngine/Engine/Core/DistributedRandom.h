#pragma once
#include <random>

namespace Glory
{
	struct RandomDevice
	{
		static std::random_device Seed;
	};

	template<typename T>
	struct DistributedRandom
	{
	public:
		explicit DistributedRandom() :
			m_Random(RandomDevice::Seed()), m_Distribution()
		{}

		explicit DistributedRandom(uint32_t seed):
			m_Random(seed), m_Distribution()
		{}

		virtual ~DistributedRandom() {}

		T Next()
		{
			return m_Distribution(m_Random);
		}

		void Reset(uint32_t seed)
		{
			m_Random.seed(seed);
		}

	private:
		std::mt19937 m_Random;
		std::uniform_int_distribution<T> m_Distribution;
	};
}
