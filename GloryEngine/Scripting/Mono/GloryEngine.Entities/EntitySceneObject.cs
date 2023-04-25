using GloryEngine.SceneManagement;
using System;
using System.Runtime.CompilerServices;

namespace GloryEngine.Entities
{
    public class EntitySceneObject : SceneObject
    {
        #region Props

        /// <summary>
        /// The EntityScene this EntitySceneObject exists in
        /// </summary>
        public EntityScene EntityScene => Scene != null ? Scene as EntityScene : null;

        /// <summary>
        /// The Entity handle for this object
        /// </summary>
        public Entity Entity
        {
            get
            {
                if (_entity.IsValid()) return _entity;
                _entity = EntitySceneObject_GetEntityHandle(_objectID, _sceneID);
                return _entity;
            }
            private set { }
        }

        #endregion

        #region Fields

        private Entity _entity;

        #endregion

        #region Constructor

        protected EntitySceneObject() { }
        protected EntitySceneObject(UInt64 objectID, UInt64 sceneID) : base(objectID, sceneID) { }

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static Entity EntitySceneObject_GetEntityHandle(UInt64 objectID, UInt64 sceneID);

        #endregion
    }
}
