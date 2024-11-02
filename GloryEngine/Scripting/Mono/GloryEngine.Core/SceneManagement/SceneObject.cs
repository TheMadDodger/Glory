using GloryEngine.Entities;
using System;
using System.Runtime.CompilerServices;

namespace GloryEngine.SceneManagement
{
    /// <summary>
    /// Handle for an entity object
    /// </summary>
    public class SceneObject : Object
    {
        #region Props

        /// <summary>
        /// The Entity handle for this object
        /// </summary>
        public Entity Entity
        {
            get
            {
                if (_entity.IsValid()) return _entity;
                _entity = SceneObject_GetEntityHandle(_objectID, _sceneID);
                return _entity;
            }
            private set { }
        }

        /// <summary>
        /// The scene this object exists in
        /// </summary>
        public virtual Scene Scene
        {
            get
            {
                if (_destroyed)
                {
                    Debug.LogError("This SceneObject has been marked for destruction.");
                    return null;
                }
                return _scene;
            }
            private set { }
        }

        /// <summary>
        /// The index of the hierarchy order relative to the parent object
        /// Or within the scene if no parent
        /// </summary>
        public uint SiblingIndex
        {
            get
            {
                if (_destroyed)
                {
                    Debug.LogError("This SceneObject has been marked for destruction.");
                    return 0;
                }
                return SceneObject_GetSiblingIndex(_objectID, _scene.ID);
            }
            set
            {
                if (_destroyed)
                {
                    Debug.LogError("This SceneObject has been marked for destruction.");
                    return;
                }
                SceneObject_SetSiblingIndex(_objectID, _scene.ID, value);
            }
        }

        /// <summary>
        /// Number of children parented to this object in the hierarchy
        /// </summary>
        public uint ChildCount
        {
            get
            {
                if (_destroyed)
                {
                    Debug.LogError("This SceneObject has been marked for destruction.");
                    return 0;
                }
                return SceneObject_GetChildCount(_objectID, _scene.ID);
            }
        }

        /// <summary>
        /// The object this object is parented to in the hierarchy
        /// Null if no parent
        /// Set to null to unparent
        /// </summary>
        public SceneObject Parent
        {
            get
            {
                if (_destroyed)
                {
                    Debug.LogError("This SceneObject has been marked for destruction.");
                    return null;
                }
                UInt64 objectID = SceneObject_GetParent(_objectID, _scene.ID);
                return objectID != 0 ? Scene.GetSceneObject(objectID) : null;
            }
            set
            {
                if (_destroyed)
                {
                    Debug.LogError("This SceneObject has been marked for destruction.");
                    return;
                }
                SceneObject_SetParent(_objectID, _scene.ID, value != null ? value.ID : 0);
            }
        }

        /// <summary>
        /// Name of the object
        /// </summary>
        public override string Name
        {
            get
            {
                if (_destroyed)
                {
                    Debug.LogError("This SceneObject has been marked for destruction.");
                    return null;
                }
                return SceneObject_GetName(_objectID, _scene.ID);
            }
            set
            {
                if (_destroyed)
                {
                    Debug.LogError("This SceneObject has been marked for destruction.");
                    return;
                }
                SceneObject_SetName(_objectID, _scene.ID, value);
            }
        }

        #endregion

        #region Fields

        private Entity _entity;
        protected UInt64 _sceneID;
        protected Scene _scene;
        internal bool _destroyed = false;

        #endregion

        #region Constructor

        internal SceneObject() { }
        internal SceneObject(UInt64 objectID, Scene scene) : base(objectID) { _scene = scene; }

        #endregion

        #region Methods

        internal void OnObjectDestroy()
        {
            _destroyed = true;
            // @todo: Destroy all components
        }

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static Entity SceneObject_GetEntityHandle(UInt64 objectID, UInt64 sceneID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static string SceneObject_GetName(UInt64 objectID, UInt64 sceneID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void SceneObject_SetName(UInt64 objectID, UInt64 sceneID, string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static uint SceneObject_GetSiblingIndex(UInt64 objectID, UInt64 sceneID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void SceneObject_SetSiblingIndex(UInt64 objectID, UInt64 sceneID, uint index);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static uint SceneObject_GetChildCount(UInt64 objectID, UInt64 sceneID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static UInt64 SceneObject_GetChild(UInt64 objectID, UInt64 sceneID, uint index);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static UInt64 SceneObject_GetParent(UInt64 objectID, UInt64 sceneID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void SceneObject_SetParent(UInt64 objectID, UInt64 sceneID, UInt64 parentID);

        #endregion
    }
}
