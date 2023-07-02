#include "ComponentTypes.h"
#include "EntityRegistry.h"
#include <exception>

namespace GloryECS
{
	ComponentTypes* ComponentTypes::m_pInstance = nullptr;
	bool ComponentTypes::m_InstanceOwned = false;

	ComponentTypes* ComponentTypes::CreateInstance()
	{
		m_InstanceOwned = true;
		m_pInstance = new ComponentTypes();
		return m_pInstance;
	}

	void ComponentTypes::DestroyInstance()
	{
		if (m_InstanceOwned) delete m_pInstance;
		m_pInstance = nullptr;
	}

	void ComponentTypes::SetInstance(ComponentTypes* pInstance)
	{
		m_pInstance = pInstance;
	}

	uint32_t ComponentTypes::GetComponentHash(const std::string& name)
	{
		if (m_pInstance->m_NameToHash.find(name) == m_pInstance->m_NameToHash.end()) return 0;
		return m_pInstance->m_NameToHash[name];
	}

	const ComponentType* ComponentTypes::GetComponentType(const uint32_t hash)
	{
		if (m_pInstance->m_ComponentTypes.find(hash) == m_pInstance->m_ComponentTypes.end()) return nullptr;
		return &m_pInstance->m_ComponentTypes.at(hash);
	}

	const size_t ComponentTypes::ComponentCount()
	{
		return m_pInstance->m_TypeHashes.size();
	}

	const ComponentType* ComponentTypes::GetComponentTypeAt(const size_t index)
	{
		if (index >= m_pInstance->m_TypeHashes.size()) return nullptr;
		const uint32_t hash = m_pInstance->m_TypeHashes[index];
		return &m_pInstance->m_ComponentTypes[hash];
	}

	void ComponentTypes::AddTypeView(std::string& name, uint32_t hash, BaseTypeView* pTypeView)
	{
		if (m_pInstance->m_pTypeViewTemplates.find(hash) != m_pInstance->m_pTypeViewTemplates.end())
			throw new std::exception("Component already registered!");

		m_pInstance->m_pTypeViewTemplates.emplace(hash, pTypeView);
		ProcessName(name);
		m_pInstance->m_NameToHash[name] = hash;
	}

	void ComponentTypes::AddComponentType(std::string& name, uint32_t hash, bool allowMultiple, uint64_t customFlags)
	{
		m_pInstance->m_ComponentTypes.emplace(hash, ComponentType{ name, hash, allowMultiple, customFlags });
	}

	BaseTypeView* ComponentTypes::CreateTypeView(EntityRegistry* pRegistry, uint32_t hash)
	{
		if (m_pInstance->m_pTypeViewTemplates.find(hash) == m_pInstance->m_pTypeViewTemplates.end())
			throw new std::exception("Component not registered!");

		BaseTypeView* pTypeView = m_pInstance->m_pTypeViewTemplates[hash]->Create(pRegistry);
		const size_t index = pRegistry->m_pViews.size();
		pRegistry->m_pViews.push_back(pTypeView);
		pRegistry->m_ViewIndices.emplace(hash, index);
		return pTypeView;
	}

	void ComponentTypes::ProcessName(std::string& name)
	{
		const char* structName = "struct ";
		const char* className = "class ";
		size_t structIndex = name.find(structName);
		if (structIndex != std::string::npos) name.erase(name.begin(), name.begin() + strlen(structName));
		size_t classIndex = name.find(className);
		if (classIndex != std::string::npos) name.erase(name.begin(), name.begin() + strlen(className));
	}

	ComponentTypes::ComponentTypes() : m_pTypeViewTemplates()
	{
	}
	
	ComponentTypes::~ComponentTypes()
	{
		for (auto it = m_pTypeViewTemplates.begin(); it != m_pTypeViewTemplates.end(); it++)
		{
			delete it->second;
		}
		m_pTypeViewTemplates.clear();
	}
}