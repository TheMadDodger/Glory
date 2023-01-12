using System;

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

        #endregion

        #region Fields

        protected Entity _entity;
        private Transform _transform = null;

        #endregion

        #region Constructor

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

        public void Initialize(Entity entity, UInt64 componentID)
        {
            _entity = entity;
            _objectID = componentID;
        }

        #endregion
    }
}
