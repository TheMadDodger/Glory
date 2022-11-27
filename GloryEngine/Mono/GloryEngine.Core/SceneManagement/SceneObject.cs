using System;
using System.Runtime.CompilerServices;

namespace GloryEngine.SceneManagement
{
    public class SceneObject : Object
    {
        #region Props

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

        public uint SiblingIndex
        {
            get => SceneObject_GetSiblingIndex(_objectID, _sceneID);
            set => SceneObject_SetSiblingIndex(_objectID, _sceneID, value);
        }

        public uint ChildCount => SceneObject_GetChildCount(_objectID, _sceneID);

        public SceneObject Parent
        {
            get
            {
                UInt64 objectID = SceneObject_GetParent(_objectID, _sceneID);
                return Scene.GetSceneObject(objectID);
            }
            set => SceneObject_SetParent(_objectID, _sceneID, value.ID);
        }

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

        public SceneObject() { }
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
