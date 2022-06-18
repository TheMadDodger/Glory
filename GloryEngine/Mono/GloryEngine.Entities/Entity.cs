using System;
using System.Runtime.CompilerServices;

namespace GloryEngine.Entities
{
    struct Entity
    {
        #region Fields

        private UInt32 _entityID;

        #endregion

        #region Constructor

        public Entity(UInt32 id)
        {
            _entityID = id;
        }

		#endregion

		#region Methods

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
