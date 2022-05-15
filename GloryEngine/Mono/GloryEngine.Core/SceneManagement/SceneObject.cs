using System.Runtime.CompilerServices;

namespace GloryEngine.SceneManagement
{
    class SceneObject : Object
    {
        #region Props

        public extern string Name
        {
            [MethodImpl(MethodImplOptions.InternalCall)]
            get;
            [MethodImpl(MethodImplOptions.InternalCall)]
            set;
        }

        public extern Scene Scene
        {
            [MethodImpl(MethodImplOptions.InternalCall)]
            get;
            [MethodImpl(MethodImplOptions.InternalCall)]
            set;
        }

        public extern uint SiblingIndex
        {
            [MethodImpl(MethodImplOptions.InternalCall)]
            get;
            [MethodImpl(MethodImplOptions.InternalCall)]
            set;
        }

        public extern uint ChildCount
        {
            [MethodImpl(MethodImplOptions.InternalCall)]
            get;
            [MethodImpl(MethodImplOptions.InternalCall)]
            private set;
        }

        public SceneObject Parent
        {
            [MethodImpl(MethodImplOptions.InternalCall)]
            get;
            [MethodImpl(MethodImplOptions.InternalCall)]
            set;
        }

        #endregion

        #region Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern SceneObject GetChild(uint index);

        #endregion
    }
}
