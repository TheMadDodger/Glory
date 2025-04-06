using System;
using System.Collections.Generic;

namespace GloryEngine.FSM
{
    internal class FSMManager
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

        #endregion
    }
}
