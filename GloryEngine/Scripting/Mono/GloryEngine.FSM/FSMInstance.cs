using System;

namespace GloryEngine.FSM
{
    /// <summary>
    /// Runtime state of a finite state machine
    /// </summary>
    public class FSMInstance
    {
        #region Props

        public UInt64 ID => _id;

        #endregion

        #region Fields

        private UInt64 _id;
        private FSMTemplate _template;

        #endregion

        #region Constructor

        internal FSMInstance(UInt64 id, FSMTemplate template)
        {
            _id = id;
            _template = template;
        }

        #endregion

        #region Methods

        public void Destroy() => FSMManager.DestroyInstance(this);

        #endregion
    }
}
