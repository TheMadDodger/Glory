using System;
using System.Runtime.CompilerServices;

namespace GloryEngine.Entities
{
    public class MeshFilter : EntityComponent
    {
        #region Props

        public Mesh Mesh
        {
            get => MeshFilter_GetMesh(ref _entity, _objectID);
            set => MeshFilter_SetMesh(ref _entity, _objectID, value != null ? value.ID : 0);
        }

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static Mesh MeshFilter_GetMesh(ref Entity entity, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void MeshFilter_SetMesh(ref Entity entity, UInt64 componentID, UInt64 meshID);

        #endregion
    }
}
