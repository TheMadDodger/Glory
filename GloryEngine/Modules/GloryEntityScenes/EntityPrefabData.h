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
        GLORY_API PrefabNode(PrefabNode&& other) noexcept;
        GLORY_API PrefabNode(EntityPrefabData* pPrefab, EntitySceneObject* pSceneObject);
        GLORY_API void operator=(EntitySceneObject* pSceneObject);
        GLORY_API void operator=(PrefabNode&& other) noexcept;

        GLORY_API const size_t ChildCount() const;
        GLORY_API const PrefabNode& ChildNode(size_t index) const;
        GLORY_API const std::string& Name() const;
        GLORY_API const std::string& SerializedComponents() const;

        GLORY_API const UUID OriginalUUID() const;
        GLORY_API const UUID TransformUUID() const;
        GLORY_API const bool ActiveSelf() const;

        GLORY_API static PrefabNode Create(EntityPrefabData* pPrefab, UUID originalUUID, UUID transformUUID, bool activeSelf, const std::string& name, const std::string& serializedComponents);
        GLORY_API PrefabNode& AddChild(EntityPrefabData* pPrefab, UUID originalUUID, UUID transformUUID, bool activeSelf, const std::string& name, const std::string& serializedComponents);

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
        GLORY_API EntityPrefabData();
        GLORY_API EntityPrefabData(PrefabNode&& rootNode) noexcept;
        GLORY_API virtual ~EntityPrefabData() = default;

        GLORY_API static EntityPrefabData* CreateFromSceneObject(EntitySceneObject* pSceneObject);

        GLORY_API const PrefabNode& RootNode() const;

        GLORY_API void SetRootNode(PrefabNode&& node);

    private:
        friend struct PrefabNode;
        PrefabNode m_RootNode;
        std::vector<UUID> m_OriginalUUIDs;
    };
}
