using System.Runtime.CompilerServices;

namespace GloryEngine.Entities
{
    /// <summary>
    /// Base class for custom Entity behaviours/scripting
    /// </summary>
    public class EntityBehaviour : Behaviour
    {
        #region Props

        /// <summary>
        /// The Entity handle that owns this component
        /// </summary>
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

        private Entity _entity = new Entity(0, 0);
        private Transform _transform = null;

        #endregion

        #region Initialization

        public void Initialize()
        {

        }

        #endregion

        #region Methods

        /// <summary>
        /// Gets a native component by type on the Entity that owns this behaviour
        /// </summary>
        /// <typeparam name="T">Type of the native component to get</typeparam>
        /// <returns>The component that matches the type, null if the Entity does not have the component</returns>
        public T GetComponent<T>() where T : EntityComponent, new()
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
