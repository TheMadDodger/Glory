using System;
using System.Collections.Generic;

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

        private Dictionary<UInt64, IFSMNodeHandler> _stateHandlers = new Dictionary<ulong, IFSMNodeHandler>();

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

        public void SetNodeHandler(FSMNode node, IFSMNodeHandler handler)
        {
            if (_stateHandlers.ContainsKey(node.ID)) _stateHandlers.Remove(node.ID);
            _stateHandlers.Add(node.ID, handler);
        }

        public void Entry(UInt64 nodeId)
        {
            if (!_stateHandlers.ContainsKey(nodeId)) return;
            _stateHandlers[nodeId].OnStateEntry(this);
        }

        public void Exit(UInt64 nodeId)
        {
            if (!_stateHandlers.ContainsKey(nodeId)) return;
            _stateHandlers[nodeId].OnStateExit(this);
        }

        #endregion
    }
}
