using System.Runtime.CompilerServices;

namespace GloryEngine.Localize
{
    /// <summary>
    /// Locale handling
    /// </summary>
    public static class Locale
    {
        #region Events

        public delegate void LanguageChangedEvent(string language);
        /// <summary>
        /// Event for language changes
        /// </summary>
        public static event LanguageChangedEvent OnLanguageChanged;

        #endregion

        #region Props

        /// <summary>
        /// Current language
        /// </summary>
        public static string Language
        {
            get => Locale_GetCurrentLanguage();
            set => Locale_SetCurrentLanguage(value);
        }

        /// <summary>
        /// Number of supported languages in this app
        /// </summary>
        public static uint LanguageCount => Locale_GetLanguageCount();

        #endregion

        #region Methods

        /// <summary>
        /// Get a supported language at a specific index
        /// </summary>
        /// <param name="index">Index of the language to get</param>
        /// <returns></returns>
        public static string GetLanguage(uint index) => Locale_GetLanguage(index);

        internal static void LanguageChanged(string language)
        {
            OnLanguageChanged?.Invoke(language);
        }

        internal static void Reset()
        {
            OnLanguageChanged = null;
        }

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static string Locale_GetCurrentLanguage();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void Locale_SetCurrentLanguage(string language);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static uint Locale_GetLanguageCount();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static string Locale_GetLanguage(uint index);

        #endregion
    }
}
