using GloryEngine.Entities;
using GloryEngine.SceneManagement;
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
        public SceneManager SceneManager => _sceneManager;

        #endregion

        #region Fields

        private AssetManager _assetManager;
        private SceneManager _sceneManager;

        #endregion

        #region Constructor

        public Engine()
        {
            _assetManager = new AssetManager(this);
            _sceneManager = new SceneManager(this);
            EntityComponentManager.Engine = this;
        }

        #endregion

        #region Methods

        internal void Reset()
        {
            _assetManager = new AssetManager(this);
            _sceneManager = new SceneManager(this);
        }

        private Resource MakeResource(UInt64 id, string typeString)
        {
            return _assetManager.MakeResource(id, typeString);
        }

        private SceneObject MakeSceneObject(UInt64 objectID, UInt64 sceneID)
        {
            Scene scene = _sceneManager.GetOpenScene(sceneID);
            if (scene == null || scene._destroyed) return null;
            return scene.GetSceneObject(objectID);
        }

        private void OnSceneDestroy(UInt64 sceneID)
        {
            _sceneManager.OnSceneDestroy(sceneID);
        }

        private void OnSceneObjectDestroy(UInt64 objectID, UInt64 sceneID)
        {
            Scene scene = _sceneManager.GetOpenScene(sceneID);
            if (scene == null || scene._destroyed) return;
            scene.OnSceneObjectDestroy(objectID);
        }

        #endregion
    }
}
