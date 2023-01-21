using GloryEngine.SceneManagement;
using System;

namespace GloryEngine.Entities
{
    public class EntitySceneObject : SceneObject
    {
        #region Props

        /// <summary>
        /// The EntityScene this EntitySceneObject exists in
        /// </summary>
        public EntityScene EntityScene => Scene != null ? Scene as EntityScene : null;

        #endregion

        #region Constructor

        protected EntitySceneObject() { }
        protected EntitySceneObject(UInt64 objectID, UInt64 sceneID) : base(objectID, sceneID) { }

        #endregion
    }
}
