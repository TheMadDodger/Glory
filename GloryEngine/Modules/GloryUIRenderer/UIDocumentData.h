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
        GLORY_API UIDocumentData();
        GLORY_API virtual ~UIDocumentData();

        template<typename T>
        UIEntity Create(UUID uuid = UUID())
        {
            const std::string_view name = T::GetTypeData()->TypeName();
            Utils::ECS::EntityID entity = CreateEntity(name, uuid);
            m_Registry.AddComponent<T>(entity);

            UIEntity uiEntity{ this, entity };
            return uiEntity;
        }

        GLORY_API Utils::ECS::EntityRegistry& GetRegistry();
        GLORY_API const std::string& Name(Utils::ECS::EntityID entity) const;
        GLORY_API Utils::ECS::EntityID CreateEmptyEntity(std::string_view name, UUID uuid = UUID());
        GLORY_API Utils::ECS::EntityID CreateEntity(std::string_view name, UUID uuid = UUID());

        GLORY_API UUID EntityUUID(Utils::ECS::EntityID entity) const;
        GLORY_API Utils::ECS::EntityID EntityID(UUID uuid) const;

    private:
        void Serialize(BinaryStream& container) const override;
        void Deserialize(BinaryStream& container) override;

    private:
        friend class UIDocument;
        friend struct UIEntity;
        Utils::ECS::EntityRegistry m_Registry;
        std::map<UUID, Utils::ECS::EntityID> m_Ids;
        std::map<Utils::ECS::EntityID, UUID> m_UUIds;
        std::map<Utils::ECS::EntityID, std::string> m_Names;
    };
}
