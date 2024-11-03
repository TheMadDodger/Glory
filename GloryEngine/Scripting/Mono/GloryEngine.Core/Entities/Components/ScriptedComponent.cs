using System;
using System.Runtime.CompilerServices;

namespace GloryEngine.Entities
{
    /// <summary>
    /// Handle for a script component
    /// You can retrieve the EntityBehaviour directly with Object.GetComponent().
    /// </summary>
    public class ScriptedComponent : NativeComponent
    {
        #region Properties

        /// <summary>
        /// The attached script asset
        /// Once set, it cannot be changed
        /// </summary>
        public Script Script
        {
            get
            {
                UInt64 scriptID = ScriptedComponent_GetScript(Object.Scene.ID, Object.ID, _objectID);
                if (scriptID == 0) return null;
                return Object.Scene.SceneManager.Engine.AssetManager.Get<Script>(scriptID);
            }
            set => ScriptedComponent_SetScript(Object.Scene.ID, Object.ID, _objectID, value != null ? value.ID : 0);
        }

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern UInt64 ScriptedComponent_GetScript(UInt64 sceneID, UInt64 objectID, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void ScriptedComponent_SetScript(UInt64 sceneID, UInt64 objectID, UInt64 componentID, UInt64 scriptID);

        #endregion
    }
}
