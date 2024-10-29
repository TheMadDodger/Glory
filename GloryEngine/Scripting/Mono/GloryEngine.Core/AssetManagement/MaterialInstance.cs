using System;

namespace GloryEngine
{
    /// <summary>
    /// Material instance handle
    /// </summary>
    public class MaterialInstance : Material
    {
        #region Constructor

        public MaterialInstance() : base() { }
        public MaterialInstance(UInt64 id) : base(id) { }

        #endregion
    }
}
