using System;

namespace GloryEngine
{
    /// <summary>
    /// Audio resource
    /// </summary>
    public class Audio : Resource
    {
        #region Constructor

        public Audio() : base() { }
        public Audio(AssetManager manager, UInt64 id) : base(manager, id) { }

        #endregion
    }
}
