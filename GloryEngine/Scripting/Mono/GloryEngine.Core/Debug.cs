using System.Runtime.CompilerServices;

namespace GloryEngine
{
	public enum LogLevel
	{
		/// <summary>
		/// Normal debug info for debugging.
		/// </summary>
		Info,
		/// <summary>
		/// A notice for things that still attention, bug fixes, changes, additions etc.
		/// </summary>
		Notice,
		/// <summary>
		/// A warning will be displayed in yellow.
		/// </summary>
		Warning,
		/// <summary>
		/// An error will be displayed in red.
		/// </summary>
		Error,
		/// <summary>
		/// A fatal error will display the error in a seperate window and then close the game.
		/// </summary>
		FatalError,
	};

	public static class Debug
    {
		[MethodImpl(MethodImplOptions.InternalCall)]
		public extern static void Log(string message, LogLevel logLevel, bool bIncludeTimeStamp = true);

		[MethodImpl(MethodImplOptions.InternalCall)]
		public extern static void LogInfo(string message, bool bIncludeTimeStamp = true);

		[MethodImpl(MethodImplOptions.InternalCall)]
		public extern static void LogNotice(string message, bool bIncludeTimeStamp = true);

		[MethodImpl(MethodImplOptions.InternalCall)]
		public extern static void LogWarning(string message, bool bIncludeTimeStamp = true);

		[MethodImpl(MethodImplOptions.InternalCall)]
		public extern static void LogError(string message, bool bIncludeTimeStamp = true);

		[MethodImpl(MethodImplOptions.InternalCall)]
		public extern static void LogFatalError(string message, bool bIncludeTimeStamp = true);

		[MethodImpl(MethodImplOptions.InternalCall)]
		public extern static void LogOnce(string key, string message, LogLevel logLevel, bool bIncludeTimeStamp = true);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static void DrawLine(Vector3 start, Vector3 end, Vector4 color, float time = 0.1f);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static void DrawRay(Vector3 start, Vector3 dir, Vector4 color, float length = 1.0f, float time = 0.1f);
    }
}
