using GloryEngine.Entities;
using System.Runtime.CompilerServices;
using System;

namespace GloryEngine.Localize
{
    /// <summary>
    /// Handle for a StringTableLoader component
    /// </summary>
    public class StringTableLoader : NativeComponent
    {
        #region Props

        /// <summary>
        /// Table to load on Start
        /// </summary>
        public StringTable TableToLoad
        {
            get
            {
                UInt64 tableID = StringTableLoader_GetTableToLoadID(Object.Scene.ID, Object.ID, _objectID);
                return tableID != 0 ? Object.Scene.SceneManager.Engine.AssetManager.Get<StringTable>(tableID) : null;
            }
            set => StringTableLoader_SetTableToLoadID(Object.Scene.ID, Object.ID, _objectID, value != null ? value.ID : 0);
        }

        /// <summary>
        /// Whether to skip unloading the table when this component is destroyed
        /// </summary>
        public bool KeepLoaded
        {
            get => StringTableLoader_GetKeepLoaded(Object.Scene.ID, Object.ID, _objectID);
            set => StringTableLoader_SetKeepLoaded(Object.Scene.ID, Object.ID, _objectID, value);
        }

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern UInt64 StringTableLoader_GetTableToLoadID(UInt64 sceneID, UInt64 objectID, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void StringTableLoader_SetTableToLoadID(UInt64 sceneID, UInt64 objectID, UInt64 componentID, UInt64 tableID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool StringTableLoader_GetKeepLoaded(UInt64 sceneID, UInt64 objectID, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void StringTableLoader_SetKeepLoaded(UInt64 sceneID, UInt64 objectID, UInt64 componentID, bool value);

        #endregion
    }
}
