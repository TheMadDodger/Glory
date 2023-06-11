#pragma once
#include <PrefabData.h>
#include <vector>

namespace Glory
{
    class EntitySceneObject;
    class EntityPrefabData;

    struct PrefabNode
    {
    public:
        PrefabNode(EntityPrefabData* pPrefab, EntitySceneObject* pSceneObject);
        void operator=(EntitySceneObject* pSceneObject);

        const size_t ChildCount() const;
        const PrefabNode& ChildNode(size_t index) const;
        const std::string& SerializedComponents() const;

        const UUID OriginalUUID() const;
        const bool ActiveSelf() const;

    private:
        std::vector<PrefabNode> m_Children;
        EntityPrefabData* m_pPrefab;
        UUID m_OriginalUUID;
        bool m_ActiveSelf;
        std::string m_SerializedComponents;

    private:
        void Load(EntitySceneObject* pSceneObject);
    };

    class EntityPrefabData : public PrefabData
    {
    public:
        EntityPrefabData();
        virtual ~EntityPrefabData() = default;

        GLORY_API static EntityPrefabData* CreateFromSceneObject(EntitySceneObject* pSceneObject);

        const PrefabNode& RootNode() const;

    private:

        PrefabNode m_RootNode;
    };
}
