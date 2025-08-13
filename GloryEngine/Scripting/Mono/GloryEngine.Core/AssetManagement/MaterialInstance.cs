using System;

namespace GloryEngine
{
    /// <summary>
    /// Material instance handle
    /// </summary>
    [System.Obsolete()]
    public class MaterialInstance : Material
    {
        #region Constructor

        public MaterialInstance() : base() { }
        public MaterialInstance(AssetManager manager, UInt64 id) : base(manager, id) { }

        #endregion
    }
}
