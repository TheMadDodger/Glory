using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace GloryEngine.FSM
{
    public class FSMManager
    {
        #region Fields

        private static Dictionary<UInt64, FSMInstance> _cachedFSMInstances = new Dictionary<UInt64, FSMInstance>();

        #endregion

        #region Constructor

        internal FSMManager()
        {
        }

        #endregion

        #region Methods

        internal void Reset()
        {
            _cachedFSMInstances.Clear();
            Debug.LogNotice("FSM Cache cleared");
        }

        internal static FSMInstance CreateInstance(UInt64 id, FSMTemplate template)
        {
            FSMInstance instance = new FSMInstance(id, template);
            _cachedFSMInstances.Add(id, instance);
            return instance;
        }

        internal static void CallNodeEntry(UInt64 fsmId, UInt64 nodeId)
        {
            if (!_cachedFSMInstances.ContainsKey(fsmId)) return;
            _cachedFSMInstances[fsmId].Entry(nodeId);
        }

        internal static void CallNodeExit(UInt64 fsmId, UInt64 nodeId)
        {
            if (!_cachedFSMInstances.ContainsKey(fsmId)) return;
            _cachedFSMInstances[fsmId].Exit(nodeId);
        }

        public static void DestroyInstance(FSMInstance instance)
        {
            if (!_cachedFSMInstances.Remove(instance.ID)) return;
            FSMManager_DestroyInstance(instance.ID);
        }

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern static void FSMManager_DestroyInstance(UInt64 instanceId);

        #endregion
    }
}
