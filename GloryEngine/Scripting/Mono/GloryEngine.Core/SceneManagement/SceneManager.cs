using System;
using System.Runtime.CompilerServices;

namespace GloryEngine.SceneManagement
{
    public static class SceneManager
    {
        #region Methods

        /// <summary>
        /// Create a new empty scene and add it to the open scenes
        /// </summary>
        /// <typeparam name="T">Type of scene that inherits from Scene</typeparam>
        /// <param name="name">Name of the new scene</param>
        /// <returns>The Scene object by type of T</returns>
        public static T CreateEmptyScene<T>(string name = "Empty Scene") where T : Scene
        {
            UInt64 sceneID = SceneManager_CreateEmptyScene(name);
            if (sceneID == 0) return null;
            return (T)Activator.CreateInstance(typeof(T), sceneID);
        }

        /// <summary>
        /// Number of open scenes
        /// </summary>
        public static uint OpenScenesCount() => SceneManager_OpenScenesCount();

        /// <summary>
        /// Get an open scene
        /// </summary>
        /// <typeparam name="T">Type to cast the open scene to</typeparam>
        /// <param name="index">Index of the open scene</param>
        /// <returns>The open scene at the specified index casted to type T or null if out of bounds</returns>
        public static T GetOpenScene<T>(uint index) where T : Scene
        {
            UInt64 sceneID = SceneManager_GetOpenScene(index);
            if (sceneID == 0) return null;
            return (T)Activator.CreateInstance(typeof(T), sceneID);
        }

        /// <summary>
        /// Get an open scene by ID
        /// </summary>
        /// <typeparam name="T">Type to cast the open scene to</typeparam>
        /// <param name="sceneID">ID of the scene</param>
        /// <returns>The scene with the corresponding ID casted to type T or null of scene is not open</returns>
        public static T GetOpenScene<T>(UInt64 sceneID) where T : Scene
        {
            sceneID = SceneManager_GetOpenSceneByUUID(sceneID);
            if (sceneID == 0) return null;
            return (T)Activator.CreateInstance(typeof(T), sceneID);
        }

        /// <summary>
        /// Gets the currently active scene
        /// </summary>
        /// <typeparam name="T">The type to cast the scene to</typeparam>
        /// <returns>The currently active scene casted to type T</returns>
        public static T GetActiveScene<T>() where T : Scene
        {
            UInt64 sceneID = SceneManager_GetActiveScene();
            if (sceneID == 0) return null;
            return (T)Activator.CreateInstance(typeof(T), sceneID);
        }

        /// <summary>
        /// Set the currently active scene
        /// Note that if the scene is not open then this call will do nothing
        /// </summary>
        /// <param name="scene">The scene to set as active scene</param>
        public static void SetActiveScene(Scene scene) => SceneManager_SetActiveScene(scene.ID);
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
        private static extern UInt64 SceneManager_CreateEmptyScene(string name = "Empty Scene");

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern uint SceneManager_OpenScenesCount();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern UInt64 SceneManager_GetOpenScene(uint index);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern UInt64 SceneManager_GetOpenSceneByUUID(UInt64 sceneID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern UInt64 SceneManager_GetActiveScene();

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
