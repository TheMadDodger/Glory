using GloryEngine.Entities;
using GloryEngine.SceneManagement;
using System;
using System.Collections.Generic;
using System.Diagnostics.Eventing.Reader;
using System.Runtime.CompilerServices;

namespace GloryEngine
{
    /// <summary>
    /// Handle to interface with the engine
    /// </summary>
    public class Engine
    {
        #region Props

        /// <summary>
        /// Resource manager
        /// </summary>
        public AssetManager AssetManager => _assetManager;
        /// <summary>
        /// Scene manager
        /// </summary>
        public SceneManager SceneManager => _sceneManager;

        /// <summary>
        /// Size of the game window
        /// </summary>
        public Vector2 WindowSize => Engine_GetWindowSize();

        /// <summary>
        /// Whether the cursor of the OS is shown
        /// </summary>
        public bool ShowWindowCursor
        {
            get => Engine_GetShowWindowCursor();
            set => Engine_SetShowWindowCursor(value);
        }

        /// <summary>
        /// Whether to claim exclusive input to the application
        /// this locks the cursor to the window.
        /// </summary>
        public bool GrabInput
        {
            get => Engine_GetGrabInput();
            set => Engine_SetGrabInput(value);
        }

        #endregion

        #region Fields

        private AssetManager _assetManager;
        private SceneManager _sceneManager;
        private List<EntityBehaviour> _scriptDummyCache;
        private List<Type> _scriptTypes;
        private Dictionary<string, Type> _resourceTypes;

        #endregion

        #region Constructor

        internal Engine()
        {
            _assetManager = new AssetManager(this);
            _sceneManager = new SceneManager(this);
            _scriptDummyCache = new List<EntityBehaviour>();
            _scriptTypes = new List<Type>();
            _resourceTypes = new Dictionary<string, Type>();

            Type baseScriptType = typeof(EntityBehaviour);
            Type baseResourceType = typeof(Resource);
            foreach (var assembly in AppDomain.CurrentDomain.GetAssemblies())
            {
                foreach (var type in assembly.GetTypes())
                {
                    if (type != baseScriptType && baseScriptType.IsAssignableFrom(type))
                    {
                        int index = _scriptTypes.Count;
                        _scriptTypes.Add(type);
                        Debug.LogNotice("Script type found " + type.Name + " for type index " + index.ToString());
                        EntityBehaviour dummy = Activator.CreateInstance(type) as EntityBehaviour;
                        _scriptDummyCache.Add(dummy);
                    }
                    else if (type != baseResourceType && baseResourceType.IsAssignableFrom(type))
                    {
                        _resourceTypes.Add(type.FullName, type);
                        Debug.LogNotice("Resource type found " + type.Name);
                    }
                }
            }
        }

        #endregion

        #region Methods

        internal void Reset()
        {
            _assetManager.Clear();
            _sceneManager.Clear();
        }

        internal Resource MakeResource(UInt64 id, string typeString)
        {
            return _assetManager.MakeResource(id, typeString);
        }

        internal Type GetResourceType(string typeString)
        {
            if (!_resourceTypes.ContainsKey(typeString)) return null;
            return _resourceTypes[typeString];
        }

        internal SceneObject MakeSceneObject(UInt64 objectID, UInt64 sceneID)
        {
            Scene scene = _sceneManager.GetOpenScene(sceneID);
            if (scene == null || scene._destroyed) return null;
            return scene.GetSceneObject(objectID);
        }

        internal void OnSceneDestroy(UInt64 sceneID)
        {
            _sceneManager.OnSceneDestroy(sceneID);
        }

        internal void OnSceneObjectDestroy(UInt64 objectID, UInt64 sceneID)
        {
            Scene scene = _sceneManager.GetOpenScene(sceneID);
            if (scene == null || scene._destroyed) return;
            scene.OnSceneObjectDestroy(objectID);
        }

        internal int GetScriptTypeCount()
        {
            return _scriptTypes.Count;
        }

        internal Type GetScriptType(int index)
        {
            return _scriptTypes[index];
        }

        internal int GetTypeIndex<T>() where T : class
        {
            Type type = typeof(T);
            for (int i = 0; i < _scriptTypes.Count; ++i)
            {
                if (_scriptTypes[i] != type) continue;
                return i;
            }
            return -1;
        }

        internal string GetScriptTypeName(int index)
        {
            return _scriptTypes[index].FullName;
        }

        internal EntityBehaviour GetScriptDummy(int index)
        {
            return _scriptDummyCache[index];
        }

        internal EntityBehaviour CreateScript(int typeIndex, UInt64 sceneID, UInt64 objectID, UInt64 componentID)
        {
            Scene scene = _sceneManager.GetOpenScene(sceneID);
            if (scene._destroyed) throw new Exception("Scene was destroyed!");
            SceneObject sceneObject = scene.GetSceneObject(objectID);
            if (sceneObject == null || sceneObject._destroyed) throw new Exception("Scene object was destroyed!");
            return sceneObject.CreateScriptComponent(GetScriptType(typeIndex), componentID);
        }

        internal EntityBehaviour GetScript(UInt64 sceneID, UInt64 objectID, UInt64 componentID)
        {
            Scene scene = _sceneManager.GetOpenScene(sceneID);
            if (scene._destroyed) throw new Exception("Scene was destroyed!");
            SceneObject sceneObject = scene.GetSceneObject(objectID);
            if (sceneObject == null || sceneObject._destroyed) throw new Exception("Scene object was destroyed!");
            return sceneObject.GetScript(componentID);
        }

        public void SetWindowCursorPos(Vector2 pos) => Engine_SetWindowCursorPos(ref pos);

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Vector2 Engine_GetWindowSize();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Engine_SetShowWindowCursor(bool show);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Engine_GetShowWindowCursor();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Engine_SetWindowCursorPos(ref Vector2 pos);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Engine_GetGrabInput();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Engine_SetGrabInput(bool grab);

        #endregion
    }
}
