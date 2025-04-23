using GloryEngine.Entities;
using System;
using System.Runtime.CompilerServices;

namespace GloryEngine.Localize
{
    /// <summary>
    /// Handle for a Localize component
    /// </summary>
    public class Localize : NativeComponent
    {
        #region Props

        /// <summary>
        /// Term to translate
        /// </summary>
        public string Term
        {
            get => Localize_GetTerm(Object.Scene.ID, Object.ID, _objectID);
            set => Localize_SetTerm(Object.Scene.ID, Object.ID, _objectID, value);
        }

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern string Localize_GetTerm(UInt64 sceneID, UInt64 objectID, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Localize_SetTerm(UInt64 sceneID, UInt64 objectID, UInt64 componentID, string term);

        #endregion
    }
}
