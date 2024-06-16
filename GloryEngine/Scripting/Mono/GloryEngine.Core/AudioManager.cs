using System;
using System.Runtime.CompilerServices;

namespace GloryEngine
{
    /// <summary>
    /// Manager for audio
    /// Requires an audio module to be loaded
    /// </summary>
    public static class AudioManager
    {
        #region Props

        /// <summary>
        /// Current master volume
        /// </summary>
        public static float MasterVolume
        {
            get => AudioManager_GetMasterVolume();
            set => AudioManager_SetMasterVolume(value);
        }

        /// <summary>
        /// Current music volume
        /// </summary>
        public static float MusicVolume
        {
            get => AudioManager_GetMusicVolume();
            set => AudioManager_SetMusicVolume(value);
        }

        #endregion

        #region Methods

        /// <summary>
        /// Play an audio asset as music
        /// </summary>
        /// <param name="audio">Audio asset to play</param>
        public static void PlayMusic(Audio audio) => AudioManager_PlayMusic(audio.ID);

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static float AudioManager_GetMasterVolume();
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void AudioManager_SetMasterVolume(float volume);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static float AudioManager_GetMusicVolume();
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void AudioManager_SetMusicVolume(float volume);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void AudioManager_PlayMusic(UInt64 audioId);

        #endregion
    }
}
