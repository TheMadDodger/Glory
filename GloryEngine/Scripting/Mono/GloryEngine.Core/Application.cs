using System.Runtime.CompilerServices;

namespace GloryEngine
{
    /// <summary>
    /// Application API
    /// </summary>
    public static class Application
    {
        #region Methods

        /// <summary>
        /// Close the application
        /// </summary>
        public static void Quit() => Application_Quit();

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Application_Quit();

        #endregion
    }
}
