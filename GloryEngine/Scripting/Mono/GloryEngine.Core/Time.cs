using System.Runtime.CompilerServices;

namespace GloryEngine
{
    /// <summary>
    /// Time
    /// </summary>
    public static class Time
    {
        #region Props

        /// <summary>
        /// The time it took to process and render the previous frame
        /// Scaled by TimeScale
        /// </summary>
        public static float DeltaTime => Time_GetDeltaTime();
        /// <summary>
        /// The time it took to process the previous frame on the game thread
        /// Scaled by TimeScale
        /// </summary>
        public static float GameDeltaTime => Time_GetGameDeltaTime();
        /// <summary>
        /// The time it took to render the previous frame on the graphics thread
        /// Scaled by TimeScale
        /// </summary>
        public static float GraphicsDeltaTime => Time_GetGraphicsDeltaTime();
        /// <summary>
        /// The unscaled time it took to process and render the previous frame
        /// </summary>
        public static float UnscaledDeltaTime => Time_GetUnscaledDeltaTime();
        /// <summary>
        /// The unscaled time it took to process the previous frame on the game thread
        /// </summary>
        public static float UnscaledGameDeltaTime => Time_GetUnscaledGameDeltaTime();
        /// <summary>
        /// The unscaled time it took to render the previous frame on the graphics thread
        /// </summary>
        public static float UnscaledGraphicsDeltaTime => Time_GetUnscaledGraphicsDeltaTime();
        /// <summary>
        /// Time elapsed since the application started in seconds
        /// Scaled by TimeScale
        /// </summary>
        public static float CurrentTime => Time_GetCurrentTime();
        /// <summary>
        /// Unsceld time elapsed since the application started in seconds
        /// </summary>
        public static float UnscaledTime => Time_GetUnscaledTime();
        /// <summary>
        /// Current frames per second
        /// </summary>
        public static float FrameRate => Time_GetFrameRate();
        /// <summary>
        /// The total number of rendered frames since the start of the application
        /// </summary>
        public static int TotalFrames => Time_GetTotalFrames();
        /// <summary>
        /// Total number of frames that were processed on the game thread since the start of the application
        /// </summary>
        public static int TotalGameFrames => Time_GetTotalGameFrames();
        /// <summary>
        /// Current scale of the time, affects all time values except unscaled ones
        /// </summary>
        public static float TimeScale
        {
            get => Time_GetTimeScale();
            set => Time_SetTimeScale(value);
        }

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static float Time_GetDeltaTime();
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static float Time_GetGameDeltaTime();
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static float Time_GetGraphicsDeltaTime();
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static float Time_GetUnscaledDeltaTime();
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static float Time_GetUnscaledGameDeltaTime();
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static float Time_GetUnscaledGraphicsDeltaTime();
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static float Time_GetCurrentTime();
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static float Time_GetUnscaledTime();
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static float Time_GetFrameRate();
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static int Time_GetTotalFrames();
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static int Time_GetTotalGameFrames();
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static float Time_GetTimeScale();
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void Time_SetTimeScale(float scale);

        #endregion
    }
}
