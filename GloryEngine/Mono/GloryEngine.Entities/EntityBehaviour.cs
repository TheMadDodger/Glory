using System.Runtime.CompilerServices;

namespace GloryEngine.Entities
{
    public class EntityBehaviour : Behaviour
    {
        #region Props

        public Entity Entity
        {
            get
            {
                if (_entity.IsValid()) return _entity;
                _entity = GetEntityHandle();
                return _entity;
            }
            private set { }
        }

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

        private Entity _entity = new Entity(0, 0);
        private Transform _transform = null;

        #endregion

        #region Initialization

        public void Initialize()
        {
            
        }

        #endregion

        #region Methods

        T GetComponent<T>() where T : EntityComponent, new()
        {
            return Entity.GetComponent<T>();
        }

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern Entity GetEntityHandle();

        #endregion
    }
}
