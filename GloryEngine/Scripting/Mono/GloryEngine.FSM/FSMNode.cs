using System;

namespace GloryEngine.FSM
{
    /// <summary>
    /// State node in a state machine
    /// </summary>
    public class FSMNode
    {
        #region Fields

        private FSMTemplate _owner;
        private UInt64 _id;

        #endregion

        #region Constructor

        internal FSMNode(FSMTemplate owner, UInt64 id)
        {
            _owner = owner;
            _id = id;
        }

        #endregion
    }
}
