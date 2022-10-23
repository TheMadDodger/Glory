using System;
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

        }

        #endregion

        #region Fields

        private Entity _entity = new Entity(0, 0);

        #endregion

        #region Initialization

        public void Initialize()
        {
            
        }

        #endregion

        #region Internal Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern Entity GetEntityHandle();

        #endregion
    }
}
