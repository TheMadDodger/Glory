#pragma once
#include <Glory.h>

#include <AssetReference.h>
#include <Reflection.h>
#include <AudioData.h>

namespace Glory
{
	struct AudioEmitter
	{
		GLORY_API AudioEmitter():
			m_AsMusic(false), m_AllowExternalEffects(false), m_AutoPlay(false), m_Loops(0) {}

		REFLECTABLE(AudioEmitter,
			(AssetReference<AudioData>)(m_Audio),
			(bool)(m_AsMusic),
			(bool)(m_AllowExternalEffects),
			(bool)(m_AutoPlay),
			(uint32_t)(m_Loops)
		)
	};
}