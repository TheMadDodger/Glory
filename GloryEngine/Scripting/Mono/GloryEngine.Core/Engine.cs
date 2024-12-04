using GloryEngine.Entities;
using GloryEngine.SceneManagement;
using System;
using System.Collections.Generic;

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

        #endregion

        #region Fields

        private AssetManager _assetManager;
        private SceneManager _sceneManager;
        private List<EntityBehaviour> _scriptDummyCache;
        private List<Type> _scriptTypes;

        #endregion

        #region Constructor

        internal Engine()
        {
            _assetManager = new AssetManager(this);
            _sceneManager = new SceneManager(this);
            _scriptDummyCache = new List<EntityBehaviour>();
            _scriptTypes = new List<Type>();

            Type baseType = typeof(EntityBehaviour);
            foreach (var assembly in AppDomain.CurrentDomain.GetAssemblies())
            {
                foreach (var type in assembly.GetTypes())
                {
                    if (type == baseType || !baseType.IsAssignableFrom(type)) continue;
                    int index = _scriptTypes.Count;
                    _scriptTypes.Add(type);
                    Debug.LogNotice("Script type found " + type.Name + " for type index " + index.ToString());
                    EntityBehaviour dummy = Activator.CreateInstance(type) as EntityBehaviour;
                    _scriptDummyCache.Add(dummy);
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

        #endregion
    }
}
