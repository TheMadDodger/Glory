using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace GloryEngine.FSM
{
    /// <summary>
    /// Finite State Machine resource
    /// </summary>
    public class FSMTemplate : Resource
    {
        #region Fields

        private Dictionary<UInt64, FSMNode> _nodeCache = new Dictionary<UInt64, FSMNode>();

        #endregion

        #region Constructor

        public FSMTemplate() : base() { }
        public FSMTemplate(AssetManager manager, UInt64 id) : base(manager, id) { }

        #endregion

        #region Methods

        public FSMInstance NewInstance()
        {
            UInt64 instanceID = FSMTemplate_CreateInstance(_objectID);
            return FSMManager.CreateInstance(instanceID, this);
        }

        public FSMNode FindNode(string name)
        {
            UInt64 nodeId = FSMTemplate_FindNode(_objectID, name);
            if (nodeId == 0) return null;
            if (_nodeCache.ContainsKey(nodeId)) return _nodeCache[nodeId];
            FSMNode node = new FSMNode(this, nodeId);
            _nodeCache.Add(nodeId, node);
            return node;
        }

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static UInt64 FSMTemplate_CreateInstance(UInt64 fsmId);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static UInt64 FSMTemplate_FindNode(UInt64 fsmId, string name);

        #endregion
    }
}
