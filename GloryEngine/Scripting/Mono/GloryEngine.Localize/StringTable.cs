using System;

namespace GloryEngine.Localize
{
    /// <summary>
    /// String table resource
    /// </summary>
    public class StringTable : Resource
    {
        #region Constructor

        public StringTable() : base() { }
        public StringTable(AssetManager manager, UInt64 id) : base(manager, id) { }

        #endregion
    }
}
