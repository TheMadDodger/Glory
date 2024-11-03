using System;
using System.Runtime.CompilerServices;

namespace GloryEngine.Entities
{
    /// <summary>
    /// Handle for a script component
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
                UInt64 scriptID = ScriptedComponent_GetScript(_object.Scene.ID, _object.ID, _objectID);
                if (scriptID == 0) return null;
                return Object.Scene.SceneManager.Engine.AssetManager.Get<Script>(scriptID);
            }
            set => ScriptedComponent_SetScript(_object.Scene.ID, _object.ID, _objectID, value != null ? value.ID : 0);
        }

        /// <summary>
        /// Get the instanced EntityBehaviour script
        /// </summary>
        //public EntityBehaviour Behaviour => ScriptedComponent_GetBehaviour(ref _entity, _objectID);

        #endregion

        #region Methods

        /// <summary>
        /// Get the casted instanced EntityBehaviour script
        /// </summary>
        /// <typeparam name="T">Type to cast to</typeparam>
        /// <returns>The casted behaviour</returns>
        //public T GetBehaviour<T>() where T : EntityBehaviour
        //{
        //    return Behaviour != null ? Behaviour as T : null;
        //}

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern UInt64 ScriptedComponent_GetScript(UInt64 sceneID, UInt64 objectID, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void ScriptedComponent_SetScript(UInt64 sceneID, UInt64 objectID, UInt64 componentID, UInt64 scriptID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern EntityBehaviour ScriptedComponent_GetBehaviour(UInt64 sceneID, UInt64 objectID, UInt64 componentID);

        #endregion
    }
}
