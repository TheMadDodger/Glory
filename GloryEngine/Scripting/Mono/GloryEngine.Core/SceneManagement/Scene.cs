using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace GloryEngine.SceneManagement
{
    /// <summary>
    /// Handle to an entity scene
    /// </summary>
    public class Scene : Object
    {
        #region Props

        /// <summary>
        /// Number of objects that exists in this Scene
        /// </summary>
        public uint ObjectsCount
        {
            get
            {
                if (_destroyed)
                {
                    throw new Exception("This Scene has been marked for destruction.");
                }
                return Scene_ObjectsCount(_objectID);
            }
        }

        /// <summary>
        /// Scene manager
        /// </summary>
        public SceneManager SceneManager => _sceneManager;
        
        #endregion

        #region Fields

        private SceneManager _sceneManager;
        private Dictionary<UInt64, SceneObject> _objectsCache = new Dictionary<UInt64, SceneObject>();
        internal bool _destroyed = false;

        #endregion

        #region Constructor

        internal Scene() { }
        internal Scene(SceneManager manager, UInt64 sceneID) : base(sceneID) { _sceneManager = manager; }

        #endregion

        #region Methods

        /// <summary>
        /// Create a new empty object in this scene
        /// </summary>
        /// <returns>The newly created object</returns>
        public SceneObject NewEmptyObject()
        {
            if (_destroyed)
            {
                throw new Exception("This Scene has been marked for destruction.");
            }
            UInt64 newObjectID = Scene_NewEmptyObject(_objectID);
            SceneObject sceneObject = new SceneObject(newObjectID, this);
            _objectsCache.Add(newObjectID, sceneObject);
            return sceneObject;
        }

        /// <summary>
        /// Create a new empty object with name in this scene
        /// </summary>
        /// <param name="name">Name to give to the object</param>
        /// <returns>The newly created object</returns>
        public SceneObject NewEmptyObject(string name)
        {
            if (_destroyed)
            {
                throw new Exception("This Scene has been marked for destruction.");
            }
            UInt64 newObjectID = Scene_NewEmptyObjectWithName(_objectID, name);
            SceneObject sceneObject = new SceneObject(newObjectID, this);
            _objectsCache.Add(newObjectID, sceneObject);
            return sceneObject;
        }

        /// <summary>
        /// Get an object in the scene
        /// </summary>
        /// <param name="index">Index of the object</param>
        [Obsolete("GetSceneObjectAt is obsolete as of 0.3.0")]
        public SceneObject GetSceneObjectAt(uint index) => null;

        /// <summary>
        /// Get an object in the scene by ID
        /// </summary>
        /// <param name="objectID">ID of the object to get</param>
        /// <returns></returns>
        public SceneObject GetSceneObject(UInt64 objectID)
        {
            if (_destroyed)
            {
                throw new Exception("This Scene has been marked for destruction.");
            }
            if (objectID == 0) return null;
            if (_objectsCache.ContainsKey(objectID)) return _objectsCache[objectID];
            SceneObject sceneObject = new SceneObject(objectID, this);
            Debug.LogInfo("Created mono scene object for ID " + objectID.ToString());
            _objectsCache.Add(objectID, sceneObject);
            return sceneObject;
        }

        /// <summary>
        /// Destroy an object in the scene
        /// </summary>
        /// <param name="sceneObject">The object to destroy</param>
        public bool Destroy(SceneObject sceneObject)
        {
            if (_destroyed)
            {
                throw new Exception("This Scene has been marked for destruction.");
            }
            if (sceneObject == null) return false;
            if (!Scene_Destroy(_objectID, sceneObject.ID)) return false;
            _objectsCache.Remove(sceneObject.ID);
            return true;
        }

        /// <summary>
        /// Spawn an instance of a prefab
        /// </summary>
        /// <param name="prefab">The prefab to spawn</param>
        /// <param name="parent">What object to parent the new instance to</param>
        /// <returns>The instance handle of the spawned prefab</returns>
        public SceneObject InstantiatePrefab(Prefab prefab, SceneObject parent = null)
        {
            if (_destroyed)
            {
                throw new Exception("This Scene has been marked for destruction.");
            }
            UInt64 newObjectID = Scene_InstantiatePrefab(_objectID, prefab.ID, new Vector3(), Quaternion.Identity, new Vector3(1, 1, 1), parent != null ? parent.ID : 0);
            SceneObject sceneObject = new SceneObject(newObjectID, this);
            _objectsCache.Add(newObjectID, sceneObject);
            return sceneObject;
        }

        /// <summary>
        /// Spawn an instance of a prefab
        /// </summary>
        /// <param name="prefab">The prefab to spawn</param>
        /// <param name="position">Local position to override the root of the prefab with</param>
        /// <param name="rotation">Local rotation to override the root of the prefab with</param>
        /// <param name="scale">Local scale to override the root of the prefab with</param>
        /// <param name="parent">What object to parent the new instance to</param>
        /// <returns></returns>
        public SceneObject InstantiatePrefab(Prefab prefab, Vector3 position, Quaternion rotation, Vector3 scale, SceneObject parent = null)
        {
            if (_destroyed)
            {
                throw new Exception("This Scene has been marked for destruction.");
            }
            UInt64 newObjectID = Scene_InstantiatePrefab(_objectID, prefab.ID, position, rotation, scale, parent != null ? parent.ID : 0);
            SceneObject sceneObject = new SceneObject(newObjectID, this);
            _objectsCache.Add(newObjectID, sceneObject);
            return sceneObject;
        }

        internal void OnSceneDestroy()
        {
            if (_destroyed) return;
            _destroyed = true;
            foreach(SceneObject sceneObject in _objectsCache.Values)
            {
                sceneObject.OnObjectDestroy();
            }
            _objectsCache.Clear();
        }

        internal void OnSceneObjectDestroy(UInt64 objectID)
        {
            if (!_objectsCache.ContainsKey(objectID)) return;
            SceneObject sceneObject = _objectsCache[objectID];
            sceneObject.OnObjectDestroy();
            _objectsCache.Remove(objectID);
        }

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern UInt64 Scene_NewEmptyObject(UInt64 sceneID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern UInt64 Scene_NewEmptyObjectWithName(UInt64 sceneID, string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern uint Scene_ObjectsCount(UInt64 sceneID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Scene_Destroy(UInt64 sceneID, UInt64 objectID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern UInt64 Scene_InstantiatePrefab(UInt64 sceneID, UInt64 prefabID, Vector3 position, Quaternion rotation, Vector3 scale, UInt64 parentID);

        #endregion
    }
}
