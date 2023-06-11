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
        PrefabNode(PrefabNode&& other) noexcept;
        PrefabNode(EntityPrefabData* pPrefab, EntitySceneObject* pSceneObject);
        void operator=(EntitySceneObject* pSceneObject);
        void operator=(PrefabNode&& other);

        const size_t ChildCount() const;
        const PrefabNode& ChildNode(size_t index) const;
        const std::string& SerializedComponents() const;

        const UUID OriginalUUID() const;
        const bool ActiveSelf() const;

        static PrefabNode Create(EntityPrefabData* pPrefab, UUID originalUUID, bool activeSelf, const std::string& serializedComponents);
        PrefabNode& AddChild(EntityPrefabData* pPrefab, UUID originalUUID, bool activeSelf, const std::string& serializedComponents);

    private:
        PrefabNode(EntityPrefabData* pPrefab, UUID originalUUID, bool activeSelf, const std::string& serializedComponents);
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
        EntityPrefabData(PrefabNode&& rootNode) noexcept;
        virtual ~EntityPrefabData() = default;

        GLORY_API static EntityPrefabData* CreateFromSceneObject(EntitySceneObject* pSceneObject);

        const PrefabNode& RootNode() const;

        void SetRootNode(PrefabNode&& node);

    private:
        PrefabNode m_RootNode;
    };
}
