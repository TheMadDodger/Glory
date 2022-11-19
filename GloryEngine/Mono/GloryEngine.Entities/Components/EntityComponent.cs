using System;

namespace GloryEngine.Entities
{
    public class EntityComponent
    {
        #region Props

        public Entity Entity => _entity;

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
        protected UInt64 _componentID;
        private Transform _transform = null;

        #endregion

        #region Constructor

        protected EntityComponent()
        {
            _entity = new Entity(0, 0);
            _componentID = 0;
        }

        #endregion

        #region Methods

        public T GetComponent<T>() where T : EntityComponent, new()
        {
            return Entity.GetComponent<T>();
        }

        public void Initialize(Entity entity, UInt64 componentID)
        {
            _entity = entity;
            _componentID = componentID;
        }

        #endregion
    }
}
