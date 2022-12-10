#pragma once
#include <Object.h>
#include <Glory.h>
#include <EntityID.h>
#include <EntityRegistry.h>

namespace Glory
{
	class EntityComponentObject : public Object
	{
	public:
		GLORY_API EntityComponentObject();
		GLORY_API EntityComponentObject(GloryECS::EntityID entityID, UUID componentID, size_t componentType, GloryECS::EntityRegistry* pRegistry);
		GLORY_API virtual ~EntityComponentObject();

		template<typename T>
		GLORY_API T& GetData()
		{
			GloryECS::TypeView<T>* pTypeView = m_pRegistry->GetTypeView<T>();
			return pTypeView->Get(m_EntityID);
		}

		GLORY_API GloryECS::EntityRegistry* GetRegistry() const;
		GLORY_API const GloryECS::EntityID EntityID() const;
		GLORY_API const size_t ComponentType() const;

		GLORY_API virtual void* GetRootDataAddress() override;

	private:
		GloryECS::EntityID m_EntityID;
		size_t m_ComponentType;
		GloryECS::EntityRegistry* m_pRegistry;
	};
}
