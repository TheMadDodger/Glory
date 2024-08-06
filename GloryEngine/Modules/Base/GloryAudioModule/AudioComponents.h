#pragma once
#include <Glory.h>

#include <AssetReference.h>
#include <Reflection.h>
#include <AudioData.h>

REFLECTABLE_ENUM_NS(Glory, SpatializationMode, Binaural, Ambisonics);
REFLECTABLE_ENUM_NS(Glory, OcclusionType, Raycast, Volumetric);
REFLECTABLE_ENUM_NS(Glory, AirAbsorptionType, Default, Exponential);

namespace Glory
{
	//Source settings:
	//
	//Spatialize X
	//	Mode[Binaural, Ambisonics] X
	//	If Ambisonics->Ambisonics Order X
	//	If Binaural->Spatial blend X
	//
	//Simulation
	//  Enabled simulation types(flags) : [Direct, Reflections, Pathing]
	//  Occlusion
	//Attenuation
	//	Min distance
	//	Max distance
	//
	//Listener settings:
	//  Enable simulation
	//    Num rays
	// 
	//Audio scene/module? settings:
	//  Scene type: Default, Embree, Radeonrays (gather options as module feature?)
	//  Simulation types(flags) : [Direct, Reflections, Pathing]
	//    If Direct enabled
	//      Attenuation
	//      Airabsorbtion
	//      Directivity
	//      Occlusion
	//      Transmission

	struct SpatializationSettings
	{
		GLORY_API SpatializationSettings() :
			m_Enable(true), m_Mode(SpatializationMode::Binaural),
			m_AmbisonicsOrder(2), m_SpatialBlend(1.0f)
		{}

		REFLECTABLE(SpatializationSettings,
			(bool)(m_Enable),
			(SpatializationMode)(m_Mode),
			(uint32_t)(m_AmbisonicsOrder),
			(float)(m_SpatialBlend)
		);
	};

	struct AttenuationSettings
	{
		GLORY_API AttenuationSettings() :
			m_Enable(true), m_MinDistance(100.0f)
		{}

		REFLECTABLE(AttenuationSettings,
			(bool)(m_Enable),
			(float)(m_MinDistance)
		);
	};

	struct AirAbsorptionSettings
	{
		GLORY_API AirAbsorptionSettings() :
			m_Enable(true), m_Type(AirAbsorptionType::Default),
			m_Coefficient1(1.0f), m_Coefficient2(1.0f), m_Coefficient3(1.0f)
		{}

		REFLECTABLE(AirAbsorptionSettings,
			(bool)(m_Enable),
			(AirAbsorptionType)(m_Type),
			(float)(m_Coefficient1),
			(float)(m_Coefficient2),
			(float)(m_Coefficient3)
		);
	};

	struct DirectivitySettings
	{
		GLORY_API DirectivitySettings() :
			m_Enable(true), m_DipoleWeight(0.5f), m_DipolePower(1.0f)
		{}

		REFLECTABLE(DirectivitySettings,
			(bool)(m_Enable),
			(float)(m_DipoleWeight),
			(float)(m_DipolePower)
		);
	};

	struct OcclusionSettings
	{
		GLORY_API OcclusionSettings() :
			m_Enable(true), m_Type(OcclusionType::Raycast),
			m_VolumetricRadius(10.0f), m_VolumetricSamples(10)
		{}

		REFLECTABLE(OcclusionSettings,
			(bool)(m_Enable),
			(OcclusionType)(m_Type),
			(float)(m_VolumetricRadius),
			(int)(m_VolumetricSamples)
		);
	};

	struct TransmissionSettings
	{
		GLORY_API TransmissionSettings() :
			m_Enable(true), m_TransmissionRays(3)
		{}

		REFLECTABLE(TransmissionSettings,
			(bool)(m_Enable),
			(int)(m_TransmissionRays)
		);
	};

	struct DirectSimulationSettings
	{
		GLORY_API DirectSimulationSettings() :
			m_Enable(true)
		{}

		REFLECTABLE(DirectSimulationSettings,
			(bool)(m_Enable),
			(AttenuationSettings)(m_DistanceAttenuation),
			(AirAbsorptionSettings)(m_AirAbsorption),
			(DirectivitySettings)(m_Directivity),
			(OcclusionSettings)(m_Occlusion),
			(TransmissionSettings)(m_Transmission)
		);
	};

	struct ReflectionSimulationSettings
	{
		GLORY_API ReflectionSimulationSettings() :
			m_Enable(true), m_ReverbScale1(1.0f), 
			m_ReverbScale2(1.0f), m_ReverbScale3(1.0f),
			m_HybridReverbTransitionTime(1.0f),
			m_HybridReverbOverlapPercent(0.25f)
		{}

		REFLECTABLE(ReflectionSimulationSettings,
			(bool)(m_Enable),
			(float)(m_ReverbScale1),
			(float)(m_ReverbScale2),
			(float)(m_ReverbScale3),
			(float)(m_HybridReverbTransitionTime),
			(float)(m_HybridReverbOverlapPercent)
		);
	};

	struct AudioSourceSimulationSettings
	{
		GLORY_API AudioSourceSimulationSettings() :
			m_Enable(true)
		{}

		REFLECTABLE(AudioSourceSimulationSettings,
			(bool)(m_Enable),
			(DirectSimulationSettings)(m_Direct),
			(ReflectionSimulationSettings)(m_Reflections)
		);
	};

	struct AudioSource
	{
		GLORY_API AudioSource():
			m_AsMusic(false), m_AutoPlay(false), m_Loops(0), m_Volume(1.0f), m_Enable3D(true) {}

		REFLECTABLE(AudioSource,
			(AssetReference<AudioData>)(m_Audio),
			(bool)(m_AsMusic),
			(bool)(m_AutoPlay),
			(uint32_t)(m_Loops),
			(float)(m_Volume),
			(bool)(m_Enable3D),
			(SpatializationSettings)(m_Spatialization),
			(AudioSourceSimulationSettings)(m_Simulation)
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