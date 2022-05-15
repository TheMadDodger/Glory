using System.Runtime.CompilerServices;

namespace GloryEngine.SceneManagement
{
    class Scene : Object
    {
        #region Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern SceneObject NewEmptyObject();

        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern uint ObjectsCount();

        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern SceneObject GetSceneObject(uint index);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern void Destroy(Object pObject);

        #endregion
    }
}
