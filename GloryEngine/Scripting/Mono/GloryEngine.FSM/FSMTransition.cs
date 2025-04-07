using System;

namespace GloryEngine.FSM
{
    /// <summary>
    /// Transition data between 2 states in a state machine
    /// </summary>
    public class FSMTransition
    {
        #region Fields

        private FSMNode _owner;
        private UInt64 _id;

        #endregion

        #region Constructor

        internal FSMTransition(FSMNode owner, UInt64 id)
        {
            _owner = owner;
            _id = id;
        }

        #endregion
    }
}
