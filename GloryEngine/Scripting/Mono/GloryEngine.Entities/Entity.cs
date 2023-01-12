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

        #endregion

        #region API Methods

        //T AddComponent<T>()
        //{
        //	return m_pEntityScene->m_Registry.AddComponent<T>(m_EntityID, std::forward<Args>(args)...);
        //}

        //template<typename T>
        //bool HasComponent()
        //{
        //	return m_pEntityScene->m_Registry.HasComponent<T>(m_EntityID);
        //}

        //template<typename T>
        //T& GetComponent()
        //{
        //	return m_pEntityScene->m_Registry.GetComponent<T>(m_EntityID);
        //}

        //template<typename T>
        //void RemoveComponent()
        //{
        //	m_pEntityScene->m_Registry.RemoveComponent<T>(m_EntityID);
        //}

        [MethodImpl(MethodImplOptions.InternalCall)]
		public extern void Clear();

		[MethodImpl(MethodImplOptions.InternalCall)]
		public extern bool IsValid();

		[MethodImpl(MethodImplOptions.InternalCall)]
		public extern void Destroy();

		//EntityScene* GetScene();

		#endregion
	}
}
