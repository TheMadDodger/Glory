using System;
using System.Runtime.CompilerServices;

namespace GloryEngine.Entities
{
    /// <summary>
    /// Handle for a ModelRenderer component
    /// </summary>
    public class ModelRenderer : NativeComponent
    {
        #region Props

        /// <summary>
        /// The first material in the material list
        /// </summary>
        public Material Material
        {
            get
            {
                UInt64 materialID = ModelRenderer_GetMaterial(Object.Scene.ID, Object.ID, _objectID);
                if (materialID == 0) return null;
                return Object.Scene.SceneManager.Engine.AssetManager.Get<Material>(materialID);
            }
            set => ModelRenderer_SetMaterial(Object.Scene.ID, Object.ID, _objectID, value != null ? value.ID : 0);
        }

        /// <summary>
        /// Number of materials on this component
        /// </summary>
        public uint MaterialCount => ModelRenderer_GetMaterialCount(Object.Scene.ID, Object.ID, _objectID);

        /// <summary>
        /// The Model to render
        /// </summary>
        public Model Model
        {
            get
            {
                UInt64 modelID = ModelRenderer_GetModel(Object.Scene.ID, Object.ID, _objectID);
                if (modelID == 1) return null;
                return Object.Scene.SceneManager.Engine.AssetManager.Get<Model>(modelID);
            }
            set => ModelRenderer_SetModel(Object.Scene.ID, Object.ID, _objectID, value != null ? value.ID : 0);
        }

        #endregion

        #region Methods

        /// <summary>
        /// Get a material from the array
        /// </summary>
        /// <param name="index">Index of the material</param>
        /// <returns>The Material object or null of out of bounds</returns>
        public Material GetMaterial(uint index)
        {
            UInt64 materialID = ModelRenderer_GetMaterialAt(Object.Scene.ID, Object.ID, _objectID, index);
            if (materialID == 0) return null;
            return Object.Scene.SceneManager.Engine.AssetManager.Get<Material>(materialID);
        }

        /// <summary>
        /// Add a material to the array
        /// </summary>
        /// <param name="material">The Material to add</param>
        public void AddMaterial(Material material)
        {
            if (material == null) return;
            ModelRenderer_AddMaterial(Object.Scene.ID, Object.ID, _objectID, material.ID);
        }

        /// <summary>
        /// Set a material at a specific index in the array
        /// </summary>
        /// <param name="index">Index to set the material</param>
        /// <param name="material">The Material to set</param>
        public void SetMaterial(uint index, Material material)
        {
            if (material == null) return;
            ModelRenderer_SetMaterialAt(Object.Scene.ID, Object.ID, _objectID, index, material.ID);
        }

        /// <summary>
        /// Clears the material array
        /// </summary>
        public void ClearMaterials()
        {
            ModelRenderer_ClearMaterials(Object.Scene.ID, Object.ID, _objectID);
        }

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static UInt64 ModelRenderer_GetMaterial(UInt64 sceneID, UInt64 objectID, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void ModelRenderer_SetMaterial(UInt64 sceneID, UInt64 objectID, UInt64 componentID, UInt64 materialID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static uint ModelRenderer_GetMaterialCount(UInt64 sceneID, UInt64 objectID, UInt64 componentID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static UInt64 ModelRenderer_GetMaterialAt(UInt64 sceneID, UInt64 objectID, UInt64 componentID, uint index);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void ModelRenderer_AddMaterial(UInt64 sceneID, UInt64 objectID, UInt64 componentID, UInt64 materialID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void ModelRenderer_SetMaterialAt(UInt64 sceneID, UInt64 objectID, UInt64 componentID, uint index, UInt64 materialID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void ModelRenderer_ClearMaterials(UInt64 sceneID, UInt64 objectID, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static UInt64 ModelRenderer_GetModel(UInt64 sceneID, UInt64 objectID, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void ModelRenderer_SetModel(UInt64 sceneID, UInt64 objectID, UInt64 componentID, UInt64 modelID);

        #endregion
    }
}
