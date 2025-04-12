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

        /// <summary>
        /// Create a new instance of this state machine
        /// </summary>
        /// <returns></returns>
        public FSMInstance NewInstance()
        {
            UInt64 instanceID = FSMTemplate_CreateInstance(_objectID);
            return FSMManager.CreateInstance(instanceID, this);
        }

        /// <summary>
        /// Find a node in this state machine
        /// </summary>
        /// <param name="name">Name of the node to find</param>
        /// <returns></returns>
        public FSMNode FindNode(string name)
        {
            UInt64 nodeId = FSMTemplate_FindNode(_objectID, name);
            if (nodeId == 0) return null;
            if (_nodeCache.ContainsKey(nodeId)) return _nodeCache[nodeId];
            FSMNode node = new FSMNode(this, nodeId);
            _nodeCache.Add(nodeId, node);
            return node;
        }

        /// <summary>
        /// Find a node in this state machine by ID
        /// </summary>
        /// <param name="nodeId">ID of the node to find</param>
        /// <returns></returns>
        public FSMNode FindNode(UInt64 nodeId)
        {
            if (_nodeCache.ContainsKey(nodeId)) return _nodeCache[nodeId];
            bool nodeExists = FSMTemplate_NodeExists(_objectID, nodeId);
            if (nodeId == 0) return null;
            FSMNode node = new FSMNode(this, nodeId);
            _nodeCache.Add(nodeId, node);
            return node;
        }

        internal FSMNode GetNode(ulong nodeId)
        {
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

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static bool FSMTemplate_NodeExists(UInt64 fsmId, UInt64 nodeID);

        #endregion
    }
}
