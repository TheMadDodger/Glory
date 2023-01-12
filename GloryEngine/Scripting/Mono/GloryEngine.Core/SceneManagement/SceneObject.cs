using System;
using System.Runtime.CompilerServices;

namespace GloryEngine.SceneManagement
{
    public class SceneObject : Object
    {
        #region Props

        /// <summary>
        /// The scene this object exists in
        /// </summary>
        public virtual Scene Scene
        {
            get
            {
                if (_scene == null)
                    _scene = SceneManager.GetOpenScene<Scene>(_sceneID);
                return _scene;
            }
            private set { }
        }

        /// <summary>
        /// The index of the hierarchy order within the parent object
        /// Or within the scene if no parent
        /// </summary>
        public uint SiblingIndex
        {
            get => SceneObject_GetSiblingIndex(_objectID, _sceneID);
            set => SceneObject_SetSiblingIndex(_objectID, _sceneID, value);
        }

        /// <summary>
        /// Number of children parented to this object in the hierarchy
        /// </summary>
        public uint ChildCount => SceneObject_GetChildCount(_objectID, _sceneID);

        /// <summary>
        /// The object this object is parented to in the hierarchy
        /// Null if no parent
        /// Set to null to unparent
        /// </summary>
        public SceneObject Parent
        {
            get
            {
                UInt64 objectID = SceneObject_GetParent(_objectID, _sceneID);
                return Scene.GetSceneObject(objectID);
            }
            set => SceneObject_SetParent(_objectID, _sceneID, value.ID);
        }

        /// <summary>
        /// Name of the object
        /// </summary>
        public override string Name
        {
            get => SceneObject_GetName(_objectID, _sceneID);
            set => SceneObject_SetName(_objectID, _sceneID, value);
        }

        #endregion

        #region Fields

        protected UInt64 _sceneID;
        protected Scene _scene;

        #endregion

        #region Constructor

        /// <summary>
        /// Default constructor
        /// Please use Scene.NewEmptyObject() instead
        /// </summary>
        public SceneObject() { }
        /// <summary>
        /// Constructor with IDs
        /// Please use Scene.NewEmptyObject() instead
        /// </summary>
        public SceneObject(UInt64 objectID, UInt64 sceneID) : base(objectID) { _sceneID = sceneID; }

        #endregion

        #region Methods

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
