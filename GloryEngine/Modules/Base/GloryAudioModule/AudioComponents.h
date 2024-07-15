#pragma once
#include <Glory.h>

#include <AssetReference.h>
#include <Reflection.h>
#include <AudioData.h>

namespace Glory
{
	struct AudioSource
	{
		GLORY_API AudioSource():
			m_AsMusic(false), m_AutoPlay(false), m_Loops(0), m_Volume(1.0f), m_Enable3D(true), m_Occlusion(true) {}

		REFLECTABLE(AudioSource,
			(AssetReference<AudioData>)(m_Audio),
			(bool)(m_AsMusic),
			(bool)(m_AutoPlay),
			(uint32_t)(m_Loops),
			(float)(m_Volume),
			(bool)(m_Enable3D),
			(bool)(m_Occlusion)
		);

		int m_CurrentChannel = -1;
	};

	struct AudioListener
	{
		GLORY_API AudioListener() {}

		REFLECTABLE(AudioListener,
			(bool)(m_Enabled)
		);
	};
}