#pragma once
#include "ECSTypeTraits.h"
#include "EntityRegistry.h"

#include <vector>
#include <memory>

namespace Glory::Utils::ECS
{
	class IComponentManager;

	class ComponentManagerFactoryBase
	{
	public:
		virtual IComponentManager* Create(EntityRegistry* pRegistry) const = 0;
	};

	template<IsComponentManager Manager>
	class ComponentManagerFactory : public ComponentManagerFactoryBase
	{
	public:
		IComponentManager* Create(EntityRegistry* pRegistry) const override
		{
			return new Manager(pRegistry);
		}
	};

	class RegistryFactory
	{
	public:
		template<IsComponentManager Manager>
		void RegisterComponentManager()
		{
			m_ComponentManagerFactories.emplace_back(new ComponentManagerFactory<Manager>());
		}

		void PopulateRegisry(EntityRegistry& registry) const
		{
			for (const auto& factory : m_ComponentManagerFactories)
				registry.AddManager(factory->Create(&registry));
		}

	private:
		std::vector<std::unique_ptr<ComponentManagerFactoryBase>> m_ComponentManagerFactories;
	};
}
