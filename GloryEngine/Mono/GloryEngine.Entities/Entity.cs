using System;
using System.Runtime.CompilerServices;

namespace GloryEngine.Entities
{
    public struct Entity
    {
		#region Props

		public UInt64 EntityID => _entityID;

		#endregion

		#region Fields

		private UInt64 _entityID;
		private UInt64 _sceneID;

        #endregion

        #region Constructor

        public Entity(UInt64 id, UInt64 sceneID)
        {
            _entityID = id;
			_sceneID = sceneID;
        }

        #endregion

        #region Methods

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
