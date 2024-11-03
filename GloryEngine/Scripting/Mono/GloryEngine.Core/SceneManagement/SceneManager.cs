using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace GloryEngine.SceneManagement
{
    /// <summary>
    /// Scene manager
    /// </summary>
    public class SceneManager
    {
        #region Props

        /// <summary>
        /// The currently active scene
        /// </summary>
        public Scene ActiveScene
        {
            get
            {
                UInt64 sceneID = SceneManager_GetActiveScene();
                if (_sceneCache.ContainsKey(sceneID))
                    return _sceneCache[sceneID];
                Scene scene = new Scene(this, sceneID);
                _sceneCache.Add(sceneID, scene);
                return scene;
            }
            set => SceneManager_SetActiveScene(value != null ? value.ID : 0);
        }

        /// <summary>
        /// Number of scenes currently open
        /// </summary>
        public uint OpenSceneCount => SceneManager_OpenScenesCount();

        /// <summary>
        /// Engine
        /// </summary>
        public Engine Engine => _engine;

        #endregion

        #region Fields

        private Engine _engine;
        private Dictionary<UInt64, Scene> _sceneCache = new Dictionary<UInt64, Scene>();

        #endregion

        #region Constructor

        internal SceneManager(Engine engine)
        {
            _engine = engine;
        }

        #endregion

        #region Methods

        /// <summary>
        /// Create a new empty scene and add it to the open scenes
        /// </summary>
        /// <param name="name">Name of the new scene</param>
        public Scene CreateEmptyScene(string name = "Empty Scene")
        {
            UInt64 sceneID = SceneManager_CreateEmptyScene(name);
            Scene newScene = new Scene(this, sceneID);
            _sceneCache.Add(sceneID, newScene);
            return newScene;
        }

        /// <summary>
        /// Get an open scene
        /// </summary>
        /// <param name="index">Index of the open scene</param>
        public Scene GetOpenScene(uint index)
        {
            UInt64 sceneID = SceneManager_GetOpenSceneAt(index);
            if (_sceneCache.ContainsKey(sceneID))
                return _sceneCache[sceneID];
            Scene scene = new Scene(this, sceneID);
            _sceneCache.Add(sceneID, scene);
            return scene;
        }

        /// <summary>
        /// Get an open scene by ID
        /// </summary>
        /// <param name="sceneID">ID of the scene</param>
        public Scene GetOpenScene(UInt64 sceneID)
        {
            if (_sceneCache.ContainsKey(sceneID))
                return _sceneCache[sceneID];
            Scene scene = new Scene(this, sceneID);
            _sceneCache.Add(sceneID, scene);
            return scene;
        }

        /// <summary>
        /// Close all currently open scenes
        /// </summary>
        public void CloseAllScenes()
        {
            foreach (Scene scene in _sceneCache.Values)
            {
                scene.OnSceneDestroy();
            }
            _sceneCache.Clear();
            SceneManager_CloseAllScenes();
        }

        /// <summary>
        /// Close a scene
        /// </summary>
        /// <param name="scene">The scene to close</param>
        public void CloseScene(Scene scene) => SceneManager_CloseScene(scene.ID);

        /// <summary>
        /// Open a scene by name
        /// </summary>
        /// <param name="name">Name of the scene to open</param>
        /// <param name="additive">Additive load, if set to true then the current loaded scenes will not be closed</param>
        public void LoadScene(string name, bool additive=false) => SceneManager_OpenSceneByName(name, additive);

        /// <summary>
        /// Open a scene by ID
        /// </summary>
        /// <param name="sceneID">ID of the scene to open</param>
        /// <param name="additive">Additive load, if set to true then the current loaded scenes will not be closed</param>
        public void LoadScene(UInt64 sceneID, bool additive=false) => SceneManager_OpenScene(sceneID, additive);

        internal void OnSceneDestroy(UInt64 sceneID)
        {
            if (!_sceneCache.ContainsKey(sceneID)) return;
            Scene scene = _sceneCache[sceneID];
            scene.OnSceneDestroy();
            _sceneCache.Remove(sceneID);
        }

        internal void Clear()
        {
            foreach (Scene scene in _sceneCache.Values)
            {
                scene.OnSceneDestroy();
            }
            _sceneCache.Clear();
        }

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern UInt64 SceneManager_CreateEmptyScene(string name = "Empty Scene");

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern uint SceneManager_OpenScenesCount();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern UInt64 SceneManager_GetOpenSceneAt(uint index);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern UInt64 SceneManager_GetOpenScene(UInt64 sceneID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern UInt64 SceneManager_GetActiveScene();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SceneManager_SetActiveScene(UInt64 sceneID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SceneManager_CloseAllScenes();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SceneManager_CloseScene(UInt64 sceneID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SceneManager_OpenSceneByName(string name, bool additive);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SceneManager_OpenScene(UInt64 sceneID, bool additive);

        #endregion
    }
}
