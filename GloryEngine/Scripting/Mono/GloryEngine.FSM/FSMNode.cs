using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace GloryEngine.FSM
{
    /// <summary>
    /// State node in a state machine
    /// </summary>
    public class FSMNode
    {
        #region Props

        public UInt64 ID => _id;

        public uint TransitionCount => FSMNode_GetTransitionCount(_owner.ID, _id);

        #endregion

        #region Fields

        private FSMTemplate _owner;
        private UInt64 _id;

        private Dictionary<UInt64, FSMTransition> _transitionCache = new Dictionary<UInt64, FSMTransition>();

        #endregion

        #region Constructor

        internal FSMNode(FSMTemplate owner, UInt64 id)
        {
            _owner = owner;
            _id = id;
        }

        #endregion

        #region Methods

        public FSMTransition FindTransition(string name)
        {
            UInt64 transitionId = FSMNode_FindTransitionID(_owner.ID, _id, name);
            if (transitionId == 0) return null;
            if (_transitionCache.ContainsKey(transitionId)) return _transitionCache[transitionId];
            FSMTransition transition = new FSMTransition(this, transitionId);
            _transitionCache.Add(transitionId, transition);
            return transition;
        }

        public FSMTransition GetTransition(uint index)
        {
            UInt64 transitionId = FSMNode_GetTransitionID(_owner.ID, _id, index);
            if (transitionId == 0) return null;
            if (_transitionCache.ContainsKey(transitionId)) return _transitionCache[transitionId];
            FSMTransition transition = new FSMTransition(this, transitionId);
            _transitionCache.Add(transitionId, transition);
            return transition;
        }

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static uint FSMNode_GetTransitionCount(UInt64 fsmId, UInt64 nodeId);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static UInt64 FSMNode_FindTransitionID(UInt64 fsmId, UInt64 nodeId, string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static UInt64 FSMNode_GetTransitionID(UInt64 fsmId, UInt64 nodeId, uint index);

        #endregion
    }
}
