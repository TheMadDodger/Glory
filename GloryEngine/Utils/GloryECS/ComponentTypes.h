#pragma once
#include "TypeView.h"
#include <map>
#include <string>
#include "../Hash.h"

namespace GloryECS
{
	class EntityRegistry;

	struct ComponentType
	{
		ComponentType()
			: m_Name("INVALID"), m_TypeHash(0), m_AllowMultiple(false), m_CustomFags(0) {}
		ComponentType(const std::string& name, const uint32_t typeHash, const bool allowMultiple, const uint64_t customFlags)
			: m_Name(name), m_TypeHash(typeHash), m_AllowMultiple(allowMultiple), m_CustomFags(customFlags) {}

		const std::string m_Name;
		const uint32_t m_TypeHash;
		const bool m_AllowMultiple;
		const uint64_t m_CustomFags;
	};

	class ComponentTypes
	{
	public:
		template<typename T>
		static void RegisterComponent(bool allowMultiple = false, const uint64_t customFlags = 0)
		{
			TypeView<T>* pTypeView = new TypeView<T>(nullptr);
			std::type_index type = typeid(T);
			uint32_t hash = Hashing::Hash(type.name());
			m_pInstance->m_TypeHashes.push_back(hash);
			std::string name = type.name();
			AddTypeView(name, hash, pTypeView);
			AddComponentType(name, hash, allowMultiple, customFlags);
		}

		static ComponentTypes* CreateInstance();
		static void DestroyInstance();
		static void SetInstance(ComponentTypes* pInstance);
		static uint32_t GetComponentHash(const std::string& name);
		static const size_t ComponentCount();
		static const ComponentType* GetComponentType(const uint32_t hash);
		static const ComponentType* GetComponentTypeAt(const size_t index);

	private:
		static void AddTypeView(std::string& name, uint32_t hash, BaseTypeView* pTypeView);
		static void AddComponentType(std::string& name, uint32_t hash, bool allowMultiple, uint64_t customFlags);

		template<typename T>
		static TypeView<T>* CreateTypeView(EntityRegistry* pRegistry)
		{
			uint32_t hash = Hashing::Hash(typeid(T).name());
			return CreateTypeView(pRegistry, hash);
		}

		static BaseTypeView* CreateTypeView(EntityRegistry* pRegistry, uint32_t hash);
		static void ProcessName(std::string& name);

	private:
		ComponentTypes();
		virtual ~ComponentTypes();

	private:
		friend class EntityRegistry;
		static ComponentTypes* m_pInstance;
		static bool m_InstanceOwned;

		std::vector<uint32_t> m_TypeHashes;
		std::map<uint32_t, BaseTypeView*> m_pTypeViewTemplates;
		std::map<std::string, uint32_t> m_NameToHash;
		std::map<uint32_t, ComponentType> m_ComponentTypes;
	};
}
