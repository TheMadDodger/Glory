using System;
using System.Runtime.CompilerServices;

namespace GloryEngine.Entities
{
    /// <summary>
    /// Handle for a MeshRenderer component
    /// </summary>
    public class MeshRenderer : EntityComponent
    {
        #region Props

        /// <summary>
        /// Material to redner the Mesh with
        /// </summary>
        public Material Material
        {
            get => MeshRenderer_GetMaterial(ref _entity, _objectID);
            set => MeshRenderer_SetMaterial(ref _entity, _objectID, value != null ? value.ID : 0);
        }

        /// <summary>
        /// The mesh to render
        /// </summary>
        public Mesh Mesh
        {
            get => MeshRenderer_GetMesh(ref _entity, _objectID);
            set => MeshRenderer_SetMesh(ref _entity, _objectID, value != null ? value.ID : 0);
        }

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static Material MeshRenderer_GetMaterial(ref Entity entity, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void MeshRenderer_SetMaterial(ref Entity entity, UInt64 componentID, UInt64 materialID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static Mesh MeshRenderer_GetMesh(ref Entity entity, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void MeshRenderer_SetMesh(ref Entity entity, UInt64 componentID, UInt64 meshID);

        #endregion
    }
}
