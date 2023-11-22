#pragma once
#include "Resource.h"

#include <EntityID.h>

namespace Glory
{
    class PrefabData;
    class GScene;
    class Entity;

    struct PrefabNode
    {
    public:
        PrefabNode(PrefabNode&& other) noexcept;
        PrefabNode(PrefabData* pPrefab, const Entity& entity);
        void operator=(const Entity& entity);
        void operator=(PrefabNode&& other) noexcept;

        const size_t ChildCount() const;
        const PrefabNode& ChildNode(size_t index) const;
        const std::string& Name() const;
        const std::string& SerializedComponents() const;

        const UUID OriginalUUID() const;
        const UUID TransformUUID() const;
        const bool ActiveSelf() const;

        static PrefabNode Create(PrefabData* pPrefab, UUID originalUUID, UUID transformUUID, bool activeSelf, const std::string& name, const std::string& serializedComponents);
        PrefabNode& AddChild(PrefabData* pPrefab, UUID originalUUID, UUID transformUUID, bool activeSelf, const std::string& name, const std::string& serializedComponents);

    private:
        void CacheOriginalUUIDs();

    private:
        friend class PrefabData;
        PrefabNode(PrefabData* pPrefab, UUID originalUUID, UUID transformUUID, bool activeSelf, const std::string& name, const std::string& serializedComponents);
        std::vector<PrefabNode> m_Children;
        PrefabData* m_pPrefab;
        UUID m_OriginalUUID;
        UUID m_TransformUUID;
        bool m_ActiveSelf;
        std::string m_Name;
        std::string m_SerializedComponents;

    private:
        void Load(const Entity& entity);
    };

    class PrefabData : public Resource
    {
    public:
        PrefabData();
        PrefabData(PrefabNode&& rootNode) noexcept;
        virtual ~PrefabData() = default;

        static PrefabData* CreateFromEntity(GScene* pScene, Utils::ECS::EntityID entity);

        const PrefabNode& RootNode() const;

        void SetRootNode(PrefabNode&& node);

    private:
        friend struct PrefabNode;
        PrefabNode m_RootNode;
        std::vector<UUID> m_OriginalUUIDs;
    };
}
