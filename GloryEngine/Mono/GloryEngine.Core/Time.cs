using System.Runtime.CompilerServices;

namespace GloryEngine
{
    public static class Time
    {
        #region Props

        public static float DeltaTime => Time_GetDeltaTime();
        public static float GameDeltaTime => Time_GetGameDeltaTime();
        public static float GraphicsDeltaTime => Time_GetGraphicsDeltaTime();
        public static float UnscaledDeltaTime => Time_GetUnscaledDeltaTime();
        public static float UnscaledGameDeltaTime => Time_GetUnscaledGameDeltaTime();
        public static float UnscaledGraphicsDeltaTime => Time_GetUnscaledGraphicsDeltaTime();
        public static float CurrentTime => Time_GetCurrentTime();
        public static float UnscaledTime => Time_GetUnscaledTime();
        public static float FrameRate => Time_GetFrameRate();
        public static int TotalFrames => Time_GetTotalFrames();
        public static int TotalGameFrames => Time_GetTotalGameFrames();
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
