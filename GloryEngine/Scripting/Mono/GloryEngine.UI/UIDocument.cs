using System;

namespace GloryEngine.UI
{
    /// <summary>
    /// UI Document resource
    /// </summary>
    public class UIDocument : Resource
    {
        #region Constructor

        public UIDocument() : base() { }
        public UIDocument(AssetManager manager, UInt64 id) : base(manager, id) { }

        #endregion
    }
}
