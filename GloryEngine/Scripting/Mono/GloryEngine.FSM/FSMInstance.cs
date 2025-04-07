using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace GloryEngine.FSM
{
    /// <summary>
    /// Runtime state of a finite state machine
    /// </summary>
    public class FSMInstance
    {
        #region Props

        public UInt64 ID => _id;

        public FSMNode CurrentState
        {
            set
            {
                if (value == null) throw new Exception("State cannot be null");
                FSMInstance_SetState(_id, value.ID);
            }
            get
            {
                UInt64 nodeId = FSMInstance_GetState(_id);
                if (nodeId == 0) return null;
                return _template.GetNode(nodeId);
            }
        }

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

        public void SetTrigger(string name) => FSMInstance_SetTrigger(_id, name);
        public void SetBool(string name, bool value) => FSMInstance_SetBool(_id, name, value);
        public void SetNumber(string name, float value) => FSMInstance_SetFloat(_id, name, value);

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void FSMInstance_SetState(UInt64 stateId, UInt64 nodeId);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static UInt64 FSMInstance_GetState(UInt64 stateId);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void FSMInstance_SetTrigger(UInt64 stateId, string name);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void FSMInstance_SetBool(UInt64 stateId, string name, bool value);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void FSMInstance_SetFloat(UInt64 stateId, string name, float value);

        #endregion
    }
}
