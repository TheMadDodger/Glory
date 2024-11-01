using GloryEngine.Entities;
using System;

namespace GloryEngine
{
    /// <summary>
    /// Handle to interface with the engine
    /// </summary>
    public class Engine
    {
        #region Props

        public AssetManager AssetManager => _assetManager;

        #endregion

        #region Fields

        private AssetManager _assetManager;

        #endregion

        #region Constructor

        public Engine()
        {
            _assetManager = new AssetManager(this);
            EntityComponentManager.Engine = this;
        }

        #endregion

        #region Methods

        internal void Reset()
        {
            _assetManager.Clear();
        }

        internal Resource MakeResource(UInt64 id, string typeString)
        {
            return _assetManager.MakeResource(id, typeString);
        }

        #endregion
    }
}
