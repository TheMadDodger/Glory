using System.Runtime.CompilerServices;
using System;

namespace GloryEngine.Entities
{
    /// <summary>
    /// Base class for custom Entity behaviours/scripting
    /// </summary>
    public class EntityBehaviour : EntityComponent
    {
        #region Props

        /// <summary>
        /// Active state of the component
        /// </summary>
        public override bool Active
        {
            get => EntityBehaviour_GetActive(Object.Scene.ID, Object.ID, _objectID);
            set => EntityBehaviour_SetActive(Object.Scene.ID, Object.ID, _objectID, value);
        }

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool EntityBehaviour_GetActive(UInt64 sceneID, UInt64 objectID, UInt64 componentID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void EntityBehaviour_SetActive(UInt64 sceneID, UInt64 objectID, UInt64 componentID, bool active);

        #endregion
    }
}
