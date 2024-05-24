using System;

namespace GloryEngine
{
    /// <summary>
    /// Material instance handle
    /// </summary>
    public class MaterialInstance : Resource
    {
        #region Constructor

        public MaterialInstance() : base() { }
        public MaterialInstance(UInt64 id) : base(id) { }

        #endregion
    }
}
