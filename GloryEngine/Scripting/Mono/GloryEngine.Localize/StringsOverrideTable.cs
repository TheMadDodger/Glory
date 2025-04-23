using System;

namespace GloryEngine.Localize
{
    /// <summary>
    /// Strings override table resource
    /// </summary>
    public class StringsOverrideTable : Resource
    {
        #region Constructor

        public StringsOverrideTable() : base() { }
        public StringsOverrideTable(AssetManager manager, UInt64 id) : base(manager, id) { }

        #endregion
    }
}
