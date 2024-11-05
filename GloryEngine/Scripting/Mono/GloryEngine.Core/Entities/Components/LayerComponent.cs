using System;
using System.Runtime.CompilerServices;

namespace GloryEngine.Entities
{
    /// <summary>
    /// Handle for a LayerComponent component
    /// </summary>
    public class LayerComponent : NativeComponent
    {
        #region Props

        /// <summary>
        /// The layer set on this component
        /// </summary>
        public Layer Layer
        {
            get => LayerComponent_GetLayer(Object.Scene.ID, Object.ID, _objectID);
            set => LayerComponent_SetLayer(Object.Scene.ID, Object.ID, _objectID, ref value);
        }

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static Layer LayerComponent_GetLayer(UInt64 sceneID, UInt64 objectID, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void LayerComponent_SetLayer(UInt64 sceneID, UInt64 objectID, UInt64 componentID, ref Layer layer);

        #endregion
    }
}
