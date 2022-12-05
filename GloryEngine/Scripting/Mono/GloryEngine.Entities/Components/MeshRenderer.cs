using System;
using System.Runtime.CompilerServices;

namespace GloryEngine.Entities
{
    public class MeshRenderer : EntityComponent
    {
        #region Props

        public Material Material
        {
            get => MeshRenderer_GetMaterial(ref _entity, _objectID);
            set => MeshRenderer_SetMaterial(ref _entity, _objectID, value != null ? value.ID : 0);
        }

        public uint MaterialCount => MeshRenderer_GetMaterialCount(ref _entity, _objectID);

        #endregion

        #region Methods

        public Material GetMaterial(uint index)
        {
            return MeshRenderer_GetMaterialAt(ref _entity, _objectID, index);
        }

        public void AddMaterial(Material material)
        {
            if (material == null) return;
            MeshRenderer_AddMaterial(ref _entity, _objectID, material.ID);
        }

        public void SetMaterial(uint index, Material material)
        {
            if (material == null) return;
            MeshRenderer_SetMaterialAt(ref _entity, _objectID, index, material.ID);
        }

        public void ClearMaterials()
        {
            MeshRenderer_ClearMaterials(ref _entity, _objectID);
        }

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static Material MeshRenderer_GetMaterial(ref Entity entity, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void MeshRenderer_SetMaterial(ref Entity entity, UInt64 componentID, UInt64 materialID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static uint MeshRenderer_GetMaterialCount(ref Entity entity, UInt64 componentID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static Material MeshRenderer_GetMaterialAt(ref Entity entity, UInt64 componentID, uint index);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void MeshRenderer_AddMaterial(ref Entity entity, UInt64 componentID, UInt64 materialID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void MeshRenderer_SetMaterialAt(ref Entity entity, UInt64 componentID, uint index, UInt64 materialID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void MeshRenderer_ClearMaterials(ref Entity entity, UInt64 componentID);

        #endregion
    }
}
