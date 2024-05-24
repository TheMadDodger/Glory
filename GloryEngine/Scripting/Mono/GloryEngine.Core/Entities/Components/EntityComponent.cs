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
        /// The Entity handle that owns this component
        /// </summary>
        public Entity Entity => _entity;

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
            get => EntityComponent_GetActive(ref _entity, _objectID);
            set => EntityComponent_SetActive(ref _entity, _objectID, value);
        }

        #endregion

        #region Fields

        /// <summary>
        /// Entity that owns this component
        /// </summary>
        protected Entity _entity;
        private Transform _transform = null;

        #endregion

        #region Constructor

        /// <summary>
        /// Construct an entity component
        /// </summary>
        protected EntityComponent()
        {
            _entity = new Entity(0, 0);
            _objectID = 0;
        }

        #endregion

        #region Methods

        /// <summary>
        /// Gets a native component by type on the Entity that owns this component
        /// </summary>
        /// <typeparam name="T">Type of the native component to get</typeparam>
        /// <returns>The component that matches the type, null if the Entity does not have the component</returns>
        public T GetComponent<T>() where T : EntityComponent, new()
        {
            return Entity.GetComponent<T>();
        }

        internal void Initialize(Entity entity, UInt64 componentID)
        {
            _entity = entity;
            _objectID = componentID;
        }

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool EntityComponent_GetActive(ref Entity entity, UInt64 componentID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void EntityComponent_SetActive(ref Entity entity, UInt64 componentID, bool active);

        #endregion
    }
}
