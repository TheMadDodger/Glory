using System;
using System.Runtime.CompilerServices;

namespace GloryEngine.Entities
{
    /// <summary>
    /// Handle for a MeshRenderer component
    /// </summary>
    public class MeshRenderer : NativeComponent
    {
        #region Props

        /// <summary>
        /// Material to redner the Mesh with
        /// </summary>
        public Material Material
        {
            get
            {
                UInt64 materialID = MeshRenderer_GetMaterial(_object.Scene.ID, _object.ID, _objectID);
                if (materialID == 0) return null;
                return Object.Scene.SceneManager.Engine.AssetManager.Get<Material>(materialID);
            }
            set => MeshRenderer_SetMaterial(_object.Scene.ID, _object.ID, _objectID, value != null ? value.ID : 0);
        }

        /// <summary>
        /// The mesh to render
        /// </summary>
        public Mesh Mesh
        {
            get
            {
                UInt64 meshID = MeshRenderer_GetMesh(_object.Scene.ID, _object.ID, _objectID);
                if (meshID == 0) return null;
                return Object.Scene.SceneManager.Engine.AssetManager.Get<Mesh>(meshID);
            }
            set => MeshRenderer_SetMesh(_object.Scene.ID, _object.ID, _objectID, value != null ? value.ID : 0);
        }

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static UInt64 MeshRenderer_GetMaterial(UInt64 sceneID, UInt64 objectID, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void MeshRenderer_SetMaterial(UInt64 sceneID, UInt64 objectID, UInt64 componentID, UInt64 materialID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static UInt64 MeshRenderer_GetMesh(UInt64 sceneID, UInt64 objectID, UInt64 componentID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void MeshRenderer_SetMesh(UInt64 sceneID, UInt64 objectID, UInt64 componentID, UInt64 meshID);

        #endregion
    }
}
