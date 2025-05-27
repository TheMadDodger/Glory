using System.Runtime.CompilerServices;

namespace GloryEngine
{
    /// <summary>
    /// Application API
    /// </summary>
    public static class Application
    {
        #region Props

        /// <summary>
        /// Current version of the application, set in the General settings of the project
        /// </summary>
        public static Version Version => Application_GetVersion();

        /// <summary>
        /// Organization name set in the general project settings
        /// </summary>
        public static string Organization => Application_GetOrganization();
        /// <summary>
        /// Application name set in the general project settings
        /// </summary>
        public static string Name => Application_GetAppName();
        /// <summary>
        /// The application preferences path
        /// This path looks like: data/Organization/AppName
        /// The data path depends on the window module, but usually looks like:
        /// - %appdata% for windows
        /// - /home/bob/.local/share for linux
        /// - /Users/bob/Library for mac os
        /// </summary>
        public static string PrefPath => Application_GetPrefPath();

        #endregion

        #region Methods

        /// <summary>
        /// Close the application
        /// </summary>
        public static void Quit() => Application_Quit();

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Application_Quit();
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static Version Application_GetVersion();
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static string Application_GetOrganization();
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static string Application_GetAppName();
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static string Application_GetPrefPath();

        #endregion
    }
}
