using System;
using System.Runtime.CompilerServices;

namespace GloryEngine.SceneManagement
{
    public static class SceneManager
    {
        #region Props

        /// <summary>
        /// The currently active scene
        /// </summary>
        public static Scene ActiveScene
        {
            get => SceneManager_GetActiveScene();
            set => SceneManager_SetActiveScene(value != null ? value.ID : 0);
        }

        /// <summary>
        /// Number of scenes currently open
        /// </summary>
        public static uint OpenSceneCount => SceneManager_OpenScenesCount();

        #endregion

        #region Methods

        /// <summary>
        /// Create a new empty scene and add it to the open scenes
        /// </summary>
        /// <param name="name">Name of the new scene</param>
        public static Scene CreateEmptyScene(string name = "Empty Scene") => SceneManager_CreateEmptyScene(name);

        /// <summary>
        /// Get an open scene
        /// </summary>
        /// <param name="index">Index of the open scene</param>
        public static Scene GetOpenScene(uint index) => SceneManager_GetOpenSceneAt(index);

        /// <summary>
        /// Get an open scene by ID
        /// </summary>
        /// <param name="sceneID">ID of the scene</param>
        public static Scene GetOpenScene(UInt64 sceneID) => SceneManager_GetOpenScene(sceneID);

        /// <summary>
        /// Close all currently open scenes
        /// </summary>
        public static void CloseAllScenes() => SceneManager_CloseAllScenes();
        /// <summary>
        /// Open a scene at a specific path
        /// </summary>
        /// <param name="path">The path to open the scene from</param>
        public static void OpenScene(string path) => SceneManager_OpenScene(path);
        /// <summary>
        /// Close a scene
        /// </summary>
        /// <param name="scene">The scene to close</param>
        public static void CloseScene(Scene scene) => SceneManager_CloseScene(scene.ID);

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Scene SceneManager_CreateEmptyScene(string name = "Empty Scene");

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern uint SceneManager_OpenScenesCount();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Scene SceneManager_GetOpenSceneAt(uint index);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Scene SceneManager_GetOpenScene(UInt64 sceneID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Scene SceneManager_GetActiveScene();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SceneManager_SetActiveScene(UInt64 sceneID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SceneManager_CloseAllScenes();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SceneManager_OpenScene(string path);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SceneManager_CloseScene(UInt64 sceneID);

        #endregion
    }
}
