using System;
using System.Runtime.CompilerServices;

namespace GloryEngine.Entities
{
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
            get => AudioSource_GetAllowEffects(ref _entity, _objectID);
            set => AudioSource_SetAllowEffects(ref _entity, _objectID, value);
        }

        /// <summary>
        /// Whether to start playing when the scene is activated
        /// </summary>
        public bool AutoPlay
        {
            get => AudioSource_GetAutoPlay(ref _entity, _objectID);
            set => AudioSource_SetAutoPlay(ref _entity, _objectID, value);
        }

        public bool Playing
        {
            get => AudioSource_GetPlaying(ref _entity, _objectID);
        }

        public bool Paused
        {
            get => AudioSource_GetPaused(ref _entity, _objectID);
            set => AudioSource_SetPaused(ref _entity, _objectID, value);
        }

        public float Volume
        {
            get => AudioSource_GetVolume(ref _entity, _objectID);
            set => AudioSource_SetVolume(ref _entity, _objectID, value);
        }

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
        private extern static bool AudioSource_GetAllowEffects(ref Entity entity, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void AudioSource_SetAllowEffects(ref Entity entity, UInt64 componentID, bool allow);

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

        #endregion
    }
}
