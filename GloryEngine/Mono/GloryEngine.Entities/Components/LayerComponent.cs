using System;
using System.Runtime.CompilerServices;

namespace GloryEngine.Entities
{
    public class LayerComponent : EntityComponent
    {
        #region Props

        public Layer Layer
        {
            get => LayerComponent_GetLayer(ref _entity, _componentID);
            set => LayerComponent_SetLayer(ref _entity, _componentID, ref value);
        }

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static Layer LayerComponent_GetLayer(ref Entity entity, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void LayerComponent_SetLayer(ref Entity entity, UInt64 componentID, ref Layer layer);

        #endregion
    }
}
