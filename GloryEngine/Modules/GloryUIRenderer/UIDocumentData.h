#pragma once
#include <Resource.h>
#include <EntityRegistry.h>
#include <TypeData.h>

namespace Glory
{
    class UIDocumentData;

    struct UIEntity
    {
        template<typename T>
        T& GetComponent()
        {
            return m_pOwner->m_Registry.GetComponent<T>(m_Entity);
        }

        UIDocumentData* m_pOwner;
        Utils::ECS::EntityID m_Entity;
    };

    class UIDocumentData : public Resource
    {
    public:
        UIDocumentData();
        virtual ~UIDocumentData();

        template<typename T>
        UIEntity Create()
        {
            const std::string_view name = T::GetTypeData()->TypeName();
            Utils::ECS::EntityID entity = CreateEntity(name);
            m_Registry.AddComponent<T>(entity);

            UIEntity uiEntity{ this, entity };
            return uiEntity;
        }

        Utils::ECS::EntityRegistry& GetRegistry();

    private:
        Utils::ECS::EntityID CreateEntity(std::string_view name);

        void Serialize(BinaryStream& container) const override;
        void Deserialize(BinaryStream& container) override;

    private:
        friend struct UIEntity;
        Utils::ECS::EntityRegistry m_Registry;
        std::map<Utils::ECS::EntityID, std::string> m_Names;
    };
}
