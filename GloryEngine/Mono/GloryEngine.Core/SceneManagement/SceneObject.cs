using System;
using System.Runtime.CompilerServices;

namespace GloryEngine.SceneManagement
{
    public class SceneObject : Object
    {
        #region Props

        public extern Scene Scene
        {
            get;
            set;
        }

        public extern uint SiblingIndex
        {
            get;
            set;
        }

        public extern uint ChildCount
        {
            get;
            private set;
        }

        public SceneObject Parent
        {
            get;
            set;
        }

        public override string Name
        {
            get => SceneObject_GetName(_objectID);
            set => SceneObject_SetName(_objectID, value);
        }

        #endregion

        #region Constructor

        public SceneObject() { }
        public SceneObject(UInt64 objectID) : base(objectID) { }

        #endregion

        #region Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern SceneObject GetChild(uint index);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static string SceneObject_GetName(UInt64 objectID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void SceneObject_SetName(UInt64 objectID, string name);

        #endregion
    }
}
