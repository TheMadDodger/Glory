//#pragma once
//#include "EntityScene.h"
//#include "EntityComponent.h"
//#include <vector>
//
//namespace Glory
//{
//	class EntityGameObject
//	{
//	public:
//		EntityGameObject(EntityScene* pScene);
//		virtual ~EntityGameObject();
//
//	public:
//		template<typename T, typename... Args>
//		T& AddComponent(Args&&... args)
//		{
//			T component = T(this, args...);
//			component.OnInitialize();
//			m_Components.push_back(component);
//			return component;
//		}
//
//		template<typename T>
//		T& GetComponent()
//		{
//			for (EntityComponent& component : m_Components)
//			{
//				T* pComp = dynamic_cast<T*>(&component);
//				if (pComp != nullptr)
//				{
//					return (T&)component;
//				}
//			}
//		}
//
//		template<typename T>
//		bool HasComponent()
//		{
//			auto it = std::find_if(m_Components.begin(), m_Components.end(),
//				[](EntityComponent& component) { return dynamic_cast<T*>(&component) != nullptr; });
//			return it != m_Components.end();
//		}
//		
//		template<typename T>
//		void RemoveComponent()
//		{
//			auto it = std::find_if(m_Components.begin(), m_Components.end(),
//				[](EntityComponent& component) { return dynamic_cast<T*>(&component) != nullptr; });
//			if (it != m_Components.end()) return;
//			m_Components.erase(it);
//		}
//
//	private:
//		void Update();
//
//	private:
//		friend class EntityScene;
//		EntityScene* m_pScene;
//		//std::vector<EntityComponent> m_Components;
//	};
//}
