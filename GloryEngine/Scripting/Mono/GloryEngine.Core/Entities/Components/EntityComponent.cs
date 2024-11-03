using GloryEngine.SceneManagement;
using System;
using System.Runtime.CompilerServices;

namespace GloryEngine.Entities
{
    /// <summary>
    /// Base class for native components
    /// </summary>
    public class EntityComponent : Object
    {
        #region Props

        /// <summary>
        /// Transform component linked to the entity that owns this component
        /// </summary>
        public Transform Transform
        {
            get
            {
                if (_transform != null) return _transform;
                _transform = GetComponent<Transform>();
                return _transform;
            }
            private set { }
        }

        /// <summary>
        /// Active state of the component
        /// </summary>
        public bool Active
        {
            get => EntityComponent_GetActive(_object.Scene.ID, _object.ID, _objectID);
            set => EntityComponent_SetActive(_object.Scene.ID, _object.ID, _objectID, value);
        }

        public SceneObject Object => _object;

        internal bool IsNative => this is NativeComponent;

        #endregion

        #region Fields

        protected SceneObject _object;
        private Transform _transform = null;
        internal bool _destroyed = false;

        #endregion

        #region Methods

        /// <summary>
        /// Gets a native component by type on the Entity that owns this component
        /// </summary>
        /// <typeparam name="T">Type of the native component to get</typeparam>
        /// <returns>The component that matches the type, null if the Entity does not have the component</returns>
        public T GetComponent<T>() where T : EntityComponent, new()
        {
            return Object.GetComponent<T>();
        }

        internal void OnComponentDestroy()
        {
            _destroyed = true;
        }

        internal void Initialize(SceneObject sceneObject, UInt64 componentID)
        {
            _object = sceneObject;
            _objectID = componentID;
        }

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool EntityComponent_GetActive(UInt64 sceneID, UInt64 objectID, UInt64 componentID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void EntityComponent_SetActive(UInt64 sceneID, UInt64 objectID, UInt64 componentID, bool active);

        #endregion
    }
}
