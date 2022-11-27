using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace GloryEngine.SceneManagement
{
    public class Scene : Object
    {
        #region Props

        public uint ObjectsCount => Scene_ObjectsCount(_objectID);

        #endregion

        #region Fields

        private Dictionary<UInt64, SceneObject> _sceneObjects = new Dictionary<UInt64, SceneObject>();

        #endregion

        #region Constructor

        public Scene() : base() { }
        public Scene(UInt64 sceneID) : base(sceneID) { }

        #endregion

        #region Methods

        public SceneObject NewEmptyObject()
        {
            UInt64 objectID = Scene_NewEmptyObject(_objectID);
            return GetNewObject(objectID);
        }

        public SceneObject NewEmptyObject(string name)
        {
            UInt64 objectID = Scene_NewEmptyObjectWithName(_objectID, name);
            return GetNewObject(objectID);
        }

        public SceneObject GetSceneObject(uint index)
        {
            UInt64 objectID = Scene_GetSceneObject(_objectID, index);
            if (objectID == 0) return null;
            if (_sceneObjects.ContainsKey(objectID)) return _sceneObjects[objectID];
            SceneObject sceneObject = CreateSceneObject(objectID);
            _sceneObjects.Add(objectID, sceneObject);
            return sceneObject;
        }

        public SceneObject GetSceneObject(UInt64 objectID)
        {
            if (objectID == 0) return null;
            if (_sceneObjects.ContainsKey(objectID)) return _sceneObjects[objectID];
            SceneObject sceneObject = CreateSceneObject(objectID);
            _sceneObjects.Add(objectID, sceneObject);
            return sceneObject;
        }

        public void Destroy(SceneObject sceneObject)
        {
            if (sceneObject == null) return;
            Scene_Destroy(_objectID, sceneObject.ID);
            _sceneObjects.Remove(sceneObject.ID);
        }

        virtual protected SceneObject CreateSceneObject(UInt64 objectID) => new SceneObject(objectID, _objectID);

        private SceneObject GetNewObject(UInt64 objectID)
        {
            if (objectID == 0) return null;
            SceneObject sceneObject = CreateSceneObject(objectID);
            _sceneObjects.Add(objectID, sceneObject);
            return sceneObject;
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
        private static extern UInt64 Scene_GetSceneObject(UInt64 sceneID, uint index);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Scene_Destroy(UInt64 sceneID, UInt64 objectID);

        #endregion
    }
}
