using System;

namespace GloryEngine
{
    /// <summary>
    /// Model handle
    /// </summary>
    public class Model : Resource
    {
        #region Constructor

        public Model() : base() { }
        public Model(AssetManager manager, UInt64 id) : base(manager, id) { }

        #endregion
    }
}
