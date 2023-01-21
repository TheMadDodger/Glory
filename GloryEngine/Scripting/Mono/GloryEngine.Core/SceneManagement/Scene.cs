using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace GloryEngine.SceneManagement
{
    public class Scene : Object
    {
        #region Props

        /// <summary>
        /// Number of objects that exists in this Scene
        /// </summary>
        public uint ObjectsCount => Scene_ObjectsCount(_objectID);

        #endregion

        #region Fields

        private Dictionary<UInt64, SceneObject> _sceneObjects = new Dictionary<UInt64, SceneObject>();

        #endregion

        #region Constructor

        protected Scene() : base() { }
        protected Scene(UInt64 sceneID) : base(sceneID) { }

        #endregion

        #region Methods

        /// <summary>
        /// Create a new empty object in this scene
        /// </summary>
        /// <returns>The newly created object</returns>
        public SceneObject NewEmptyObject() => Scene_NewEmptyObject(_objectID);

        /// <summary>
        /// Create a new empty object with name in this scene
        /// </summary>
        /// <param name="name">Name to give to the object</param>
        /// <returns>The newly created object</returns>
        public SceneObject NewEmptyObject(string name) => Scene_NewEmptyObjectWithName(_objectID, name);

        /// <summary>
        /// Get an object in the scene
        /// </summary>
        /// <param name="index">Index of the object</param>
        public SceneObject GetSceneObjectAt(uint index) => Scene_GetSceneObjectAt(_objectID, index);

        /// <summary>
        /// Get an object in the scene by ID
        /// </summary>
        /// <param name="objectID">ID of the object to get</param>
        /// <returns></returns>
        public SceneObject GetSceneObject(UInt64 objectID)
        {
            if (objectID == 0) return null;
            if (_sceneObjects.ContainsKey(objectID)) return _sceneObjects[objectID];
            SceneObject sceneObject = Scene_GetSceneObject(_objectID, objectID);
            if (sceneObject == null) return null;
            _sceneObjects.Add(objectID, sceneObject);
            return sceneObject;
        }

        /// <summary>
        /// Destroy an object in the scene
        /// </summary>
        /// <param name="sceneObject">The object to destroy</param>
        public bool Destroy(SceneObject sceneObject)
        {
            if (sceneObject == null) return false;
            if (!Scene_Destroy(_objectID, sceneObject.ID)) return false;
            _sceneObjects.Remove(sceneObject.ID);
            return true;
        }

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern SceneObject Scene_NewEmptyObject(UInt64 sceneID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern SceneObject Scene_NewEmptyObjectWithName(UInt64 sceneID, string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern uint Scene_ObjectsCount(UInt64 sceneID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern SceneObject Scene_GetSceneObject(UInt64 sceneID, UInt64 objectID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern SceneObject Scene_GetSceneObjectAt(UInt64 sceneID, uint index);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Scene_Destroy(UInt64 sceneID, UInt64 objectID);

        #endregion
    }
}
