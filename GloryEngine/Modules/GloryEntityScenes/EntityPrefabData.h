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
        void operator=(PrefabNode&& other) noexcept;

        const size_t ChildCount() const;
        const PrefabNode& ChildNode(size_t index) const;
        const std::string& Name() const;
        const std::string& SerializedComponents() const;

        const UUID OriginalUUID() const;
        const UUID TransformUUID() const;
        const bool ActiveSelf() const;

        static PrefabNode Create(EntityPrefabData* pPrefab, UUID originalUUID, UUID transformUUID, bool activeSelf, const std::string& name, const std::string& serializedComponents);
        PrefabNode& AddChild(EntityPrefabData* pPrefab, UUID originalUUID, UUID transformUUID, bool activeSelf, const std::string& name, const std::string& serializedComponents);

    private:
        void CacheOriginalUUIDs();

    private:
        friend class EntityPrefabData;
        PrefabNode(EntityPrefabData* pPrefab, UUID originalUUID, UUID transformUUID, bool activeSelf, const std::string& name, const std::string& serializedComponents);
        std::vector<PrefabNode> m_Children;
        EntityPrefabData* m_pPrefab;
        UUID m_OriginalUUID;
        UUID m_TransformUUID;
        bool m_ActiveSelf;
        std::string m_Name;
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

        void GenerateNewUUIDs(std::map<UUID, UUID>& uuidRemap) const;

    private:
        friend struct PrefabNode;
        PrefabNode m_RootNode;
        std::vector<UUID> m_OriginalUUIDs;
    };
}
