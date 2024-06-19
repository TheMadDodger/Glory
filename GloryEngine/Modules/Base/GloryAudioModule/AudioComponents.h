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
			m_AsMusic(false), m_AllowExternalEffects(false), m_AutoPlay(false), m_Loops(0), m_Volume(1.0f) {}

		REFLECTABLE(AudioSource,
			(AssetReference<AudioData>)(m_Audio),
			(bool)(m_AsMusic),
			(bool)(m_AllowExternalEffects),
			(bool)(m_AutoPlay),
			(uint32_t)(m_Loops),
			(float)(m_Volume)
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