using System;
using System.Runtime.CompilerServices;

namespace GloryEngine.SceneManagement
{
    public static class SceneManager
    {
        #region Methods

        public static T CreateEmptyScene<T>(string name = "Empty Scene") where T : Scene
        {
            UInt64 sceneID = SceneManager_CreateEmptyScene(name);
            if (sceneID == 0) return null;
            return (T)Activator.CreateInstance(typeof(T), sceneID);
        }

        public static uint OpenScenesCount() => SceneManager_OpenScenesCount();

        public static T GetOpenScene<T>(uint index) where T : Scene
        {
            UInt64 sceneID = SceneManager_GetOpenScene(index);
            if (sceneID == 0) return null;
            return (T)Activator.CreateInstance(typeof(T), sceneID);
        }

        public static T GetOpenScene<T>(UInt64 sceneID) where T : Scene
        {
            sceneID = SceneManager_GetOpenSceneByUUID(sceneID);
            if (sceneID == 0) return null;
            return (T)Activator.CreateInstance(typeof(T), sceneID);
        }

        public static T GetActiveScene<T>() where T : Scene
        {
            UInt64 sceneID = SceneManager_GetActiveScene();
            if (sceneID == 0) return null;
            return (T)Activator.CreateInstance(typeof(T), sceneID);
        }

        public static void SetActiveScene(Scene scene) => SceneManager_SetActiveScene(scene.ID);
        public static void CloseAllScenes() => SceneManager_CloseAllScenes();
        public static void OpenScene(string path) => SceneManager_OpenScene(path);
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
