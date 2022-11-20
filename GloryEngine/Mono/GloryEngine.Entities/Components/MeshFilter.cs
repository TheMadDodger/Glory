using System;
using System.Runtime.CompilerServices;

namespace GloryEngine.Entities
{
    public class MeshFilter : EntityComponent
    {
        #region Props

        public Model Model
        {
            get => MeshFilter_GetModel(ref _entity, _objectID);
            set => MeshFilter_SetModel(ref _entity, _objectID, value != null ? value.ID : 0);
        }

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static Model MeshFilter_GetModel(ref Entity entity, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void MeshFilter_SetModel(ref Entity entity, UInt64 componentID, UInt64 modelID);

        #endregion
    }
}
