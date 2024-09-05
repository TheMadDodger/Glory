using System;
using System.Runtime.CompilerServices;

namespace GloryEngine.Entities
{
    /// <summary>
    /// Spatialization mode
    /// </summary>
    public enum SpatializationMode
    {
        Binaural,
        Ambisonics
    }

    /// <summary>
    /// Ambisonics order
    /// </summary>
    public enum AmbisonicsOrder
    {
        First,
        Second,
        Third
    }

    /// <summary>
    /// Occlusion type
    /// </summary>
    public enum OcclusionType
    {
        Raycast,
        Volumetric
    }

    /// <summary>
    /// Air absorption type
    /// </summary>
    public enum AirAbsorptionType
    {
        Default,
        Exponential
    }

    /// <summary>
    /// Attenuation settings
    /// </summary>
    public struct AttenuationSettings
    {
        public bool Enable;
        public float MinDistance;
    };

    /// <summary>
    /// Spatialization settings
    /// </summary>
    public struct SpatializationSettings
    {
        public bool Enable;
        public SpatializationMode Mode;
        public AmbisonicsOrder AmbisonicsOrder;
        public float SpatialBlend;
        public AttenuationSettings Attenuation;
    };

    /// <summary>
    /// Air absorption settings
    /// </summary>
    public struct AirAbsorptionSettings
    {
        public bool Enable;
        public AirAbsorptionType Type;
        public float LowCoefficient;
        public float MidCoefficient;
        public float HighCoefficient;
    };

    /// <summary>
    /// Directivity settings
    /// </summary>
    public struct DirectivitySettings
    {
        public bool Enable;
        public float DipoleWeight;
        public float DipolePowe;
    };

    /// <summary>
    /// Occlusion settings
    /// </summary>
    public struct OcclusionSettings
    {
        public bool Enable;
        public OcclusionType Type;
        public float VolumetricRadius;
        public int VolumetricSamples;
    };

    /// <summary>
    /// Transmission settings
    /// </summary>
    public struct TransmissionSettings
    {
        bool Enable;
        int TransmissionRays;
        bool FrequencyDependan;
    };

    /// <summary>
    /// Direct simulation settings
    /// </summary>
    public struct DirectSimulationSettings
    {
        public bool m_Enable;
        public AttenuationSettings DistanceAttenuation;
        public AirAbsorptionSettings AirAbsorption;
        public DirectivitySettings Directivity;
        public OcclusionSettings Occlusion;
        public TransmissionSettings Transmission;
    };

    /// <summary>
    /// Reflection simulation settings
    /// </summary>
    public struct ReflectionSimulationSettings
    {
        public bool Enable;
        public float ReverbScale1;
        public float ReverbScale2;
        public float ReverbScale3;
        public float HybridReverbTransitionTime;
        public float HybridReverbOverlapPercent;
    };

    /// <summary>
    /// Pathing simulation settings
    /// </summary>
    public struct PathingSimulationSettings
    {
        public bool Enable;
        public float VisRadius;
        public int PathingOrder;
        public bool Validation;
        public bool FindAlternativePaths;
    };

    /// <summary>
    /// Simulation settings
    /// </summary>
    public struct AudioSourceSimulationSettings
    {
        public bool Enable;
        public DirectSimulationSettings Direct;
        public ReflectionSimulationSettings Reflections;
        public PathingSimulationSettings Pathing;
    }

    /// <summary>
    /// Handle for an AudioSource component
    /// </summary>
    public class AudioSource : EntityComponent
    {
        #region Props

        /// <summary>
        /// Audio asset that this emitter plays
        /// </summary>
        public Audio Audio
        {
            get => AudioSource_GetAudio(ref _entity, _objectID);
            set => AudioSource_SetAudio(ref _entity, _objectID, value != null ? value.ID : 0);
        }

        /// <summary>
        /// Whether to play this audio on the music channel
        /// </summary>
        public bool AsMusic
        {
            get => AudioSource_GetAsMusic(ref _entity, _objectID);
            set => AudioSource_SetAsMusic(ref _entity, _objectID, value);
        }

        /// <summary>
        /// How many times the audio should be looped
        /// </summary>
        public UInt32 Loops
        {
            get => AudioSource_GetLoops(ref _entity, _objectID);
            set => AudioSource_SetLoops(ref _entity, _objectID, value);
        }

        /// <summary>
        /// Whether to allow effect processing on this emitter
        /// </summary>
        public bool AllowEffects
        {
            get => AudioSource_GetEnable3D(ref _entity, _objectID);
            set => AudioSource_SetEnable3D(ref _entity, _objectID, value);
        }

        /// <summary>
        /// Whether to start playing when the scene is activated
        /// </summary>
        public bool AutoPlay
        {
            get => AudioSource_GetAutoPlay(ref _entity, _objectID);
            set => AudioSource_SetAutoPlay(ref _entity, _objectID, value);
        }

        /// <summary>
        /// Whether the source is currently playing
        /// </summary>
        public bool Playing
        {
            get => AudioSource_GetPlaying(ref _entity, _objectID);
        }

        /// <summary>
        /// Whether the source is currently paused
        /// </summary>
        public bool Paused
        {
            get => AudioSource_GetPaused(ref _entity, _objectID);
            set => AudioSource_SetPaused(ref _entity, _objectID, value);
        }

        /// <summary>
        /// Current volume of the source
        /// </summary>
        public float Volume
        {
            get => AudioSource_GetVolume(ref _entity, _objectID);
            set => AudioSource_SetVolume(ref _entity, _objectID, value);
        }

        /// <summary>
        /// Spatialization settings
        /// </summary>
        public ref SpatializationSettings SpatializationSettings => ref AudioSource_GetSpatializationSettings(ref _entity, _objectID);

        /// <summary>
        /// Simulation settings
        /// </summary>
        public ref AudioSourceSimulationSettings SimulationSettings => ref AudioSource_GetSimulationSettings(ref _entity, _objectID);

        #endregion

        #region Methods

        /// <summary>
        /// Play or resume the sound on the emitter
        /// </summary>
        public void Play() => AudioSource_Play(ref _entity, _objectID);
        /// <summary>
        /// Stop the sound on the emitter
        /// </summary>
        public void Stop() => AudioSource_Stop(ref _entity, _objectID);
        /// <summary>
        /// Pause the sound on the emitter
        /// </summary>
        public void Pause() => AudioSource_Pause(ref _entity, _objectID);

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static Audio AudioSource_GetAudio(ref Entity entity, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void AudioSource_SetAudio(ref Entity entity, UInt64 componentID, UInt64 audioID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static bool AudioSource_GetAsMusic(ref Entity entity, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void AudioSource_SetAsMusic(ref Entity entity, UInt64 componentID, bool asMusic);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static UInt32 AudioSource_GetLoops(ref Entity entity, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void AudioSource_SetLoops(ref Entity entity, UInt64 componentID, UInt32 loops);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static bool AudioSource_GetEnable3D(ref Entity entity, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void AudioSource_SetEnable3D(ref Entity entity, UInt64 componentID, bool allow);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static bool AudioSource_GetAutoPlay(ref Entity entity, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void AudioSource_SetAutoPlay(ref Entity entity, UInt64 componentID, bool autoPlay);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static bool AudioSource_GetPlaying(ref Entity entity, UInt64 componentID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static bool AudioSource_GetPaused(ref Entity entity, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void AudioSource_SetPaused(ref Entity entity, UInt64 componentID, bool autoPlay);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static float AudioSource_GetVolume(ref Entity entity, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void AudioSource_SetVolume(ref Entity entity, UInt64 componentID, float volume);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void AudioSource_Play(ref Entity entity, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void AudioSource_Stop(ref Entity entity, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void AudioSource_Pause(ref Entity entity, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void AudioSource_Resume(ref Entity entity, UInt64 componentID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static ref SpatializationSettings AudioSource_GetSpatializationSettings(ref Entity entity, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void AudioSource_SetSpatializationSettings(ref Entity entity, UInt64 componentID, ref SpatializationSettings value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static ref AudioSourceSimulationSettings AudioSource_GetSimulationSettings(ref Entity entity, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void AudioSource_SetSimulationSettings(ref Entity entity, UInt64 componentID, ref AudioSourceSimulationSettings value);

        #endregion
    }
}
