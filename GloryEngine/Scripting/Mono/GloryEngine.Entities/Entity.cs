using System;
using System.Runtime.CompilerServices;

namespace GloryEngine.Entities
{
    /// <summary>
    /// Handle for an Entity
    /// </summary>
    public struct Entity
    {
		#region Props

        /// <summary>
        /// ID of the Entity
        /// </summary>
		public UInt64 EntityID => _entityID;
        
        /// <summary>
        /// ID of the scene the Entity exists in
        /// </summary>
		public UInt64 SceneID => _sceneID;

        /// <summary>
        /// Object in the scene that owns this Entity
        /// </summary>
        public EntitySceneObject SceneObject => Entity_GetSceneObjectID(this);

        public EntityScene Scene => SceneManagement.SceneManager.GetOpenScene(_sceneID) as EntityScene;

        #endregion

        #region Fields

        private readonly UInt64 _entityID;
		private readonly UInt64 _sceneID;

        #endregion

        #region Constructor

        public Entity(UInt64 id, UInt64 sceneID)
        {
            _entityID = id;
			_sceneID = sceneID;
        }

        #endregion

        #region Methods

        /// <summary>
        /// Gets a native component by type on the Entity that owns this component
        /// </summary>
        /// <typeparam name="T">Type of the native component to get</typeparam>
        /// <returns>The component that matches the type, null if the Entity does not have the component</returns>
        public T GetComponent<T>() where T : EntityComponent, new()
        {
            return EntityComponentManager.GetComponent<T>(ref this);
        }

        /// <summary>
        /// Check whether this Entity is valid
        /// </summary>
        public bool IsValid() => Entity_IsValid(this);

        /// <summary>
        /// Add a new component to this entity
        /// </summary>
        /// <typeparam name="T">Type of component to add</typeparam>
        /// <returns>The newly constructed component or null if failed</returns>
        public T AddComponent<T>() where T : EntityComponent, new()
            => EntityComponentManager.AddComponent<T>(ref this);

        /// <summary>
        /// Remove a component from this entity
        /// </summary>
        /// <typeparam name="T">The type of component to remove</typeparam>
        public void RemoveComponent<T>() where T : EntityComponent, new()
            => EntityComponentManager.RemoveComponent<T>(ref this);

        /// <summary>
        /// Check if this entity has certain component
        /// </summary>
        /// <typeparam name="T">The type of component to check for</typeparam>
        /// <returns>True of the entity has the component</returns>
        public bool HasComponent<T>() where T : EntityComponent, new()
            => EntityComponentManager.HasComponent<T>(ref this);

        #endregion

        #region API Methods

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Entity_IsValid(Entity entity);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern EntitySceneObject Entity_GetSceneObjectID(Entity entity);

        #endregion
    }
}
