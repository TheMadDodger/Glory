using System;
using GloryEngine.SceneManagement;

namespace GloryEngine.Entities
{
    /// <summary>
    /// Handle to an entity scene
    /// </summary>
    public class EntityScene : Scene
    {
        #region Constructor

        protected EntityScene() { }
        protected EntityScene(UInt64 sceneID) : base(sceneID) { }

        #endregion

        #region Methods

        /// <summary>
        /// Create a new empty entity object in this scene
        /// </summary>
        /// <returns>The newly created object</returns>
        public EntitySceneObject NewEntityObject() => NewEmptyObject() as EntitySceneObject;

        /// <summary>
        /// Create a new empty entity object with name in this scene
        /// </summary>
        /// <param name="name">Name to give to the object</param>
        /// <returns>The newly created object</returns>
        public EntitySceneObject NewEntityObject(string name) => NewEmptyObject(name) as EntitySceneObject;

        /// <summary>
        /// Get an entity object in the scene
        /// </summary>
        /// <param name="index">Index of the object</param>
        public EntitySceneObject GetEntitySceneObjectAt(uint index) => GetSceneObjectAt(index) as EntitySceneObject;

        /// <summary>
        /// Get an entity object in the scene by ID
        /// </summary>
        /// <param name="objectID">ID of the object to get</param>
        /// <returns></returns>
        public EntitySceneObject GetEntitySceneObject(UInt64 objectID) => GetSceneObject(objectID) as EntitySceneObject;

        /// <summary>
        /// Destroy an entity object in the scene
        /// </summary>
        /// <param name="sceneObject">The object to destroy</param>
        public bool DestroyEntity(EntitySceneObject sceneObject) => Destroy(sceneObject);

        /// <summary>
        /// Spawn an entity instance of a prefab
        /// </summary>
        /// <param name="prefab">The prefab to spawn</param>
        /// <param name="parent">What object to parent the new instance to</param>
        /// <returns>The instance handle of the spawned prefab</returns>
        public EntitySceneObject InstantiateEntityPrefab(EntityPrefab prefab, SceneObject parent = null)
            => InstantiatePrefab(prefab, parent) as EntitySceneObject;

        /// <summary>
        /// Spawn an entity instance of a prefab
        /// </summary>
        /// <param name="prefab">The prefab to spawn</param>
        /// <param name="position">Local position to override the root of the prefab with</param>
        /// <param name="rotation">Local rotation to override the root of the prefab with</param>
        /// <param name="scale">Local scale to override the root of the prefab with</param>
        /// <param name="parent">What object to parent the new instance to</param>
        /// <returns></returns>
        public EntitySceneObject InstantiateEntityPrefab(EntityPrefab prefab, Vector3 position, Quaternion rotation, Vector3 scale, SceneObject parent = null)
            => InstantiatePrefab(prefab, position, rotation, scale, parent) as EntitySceneObject;

        #endregion
    }
}
