//#pragma once
//#include "Resource.h"
//#include "SceneObject.h"
//#include "UUIDRemapper.h"
//#include "Entity.h"
//
//#include <mutex>
//#include <glm/fwd.hpp>
//#include <EntityRegistry.h>
//
//namespace Glory
//{
//    struct PrefabNode;
//    class PrefabData;
//    class SceneObject;
//
//    struct DelayedParentData
//    {
//        DelayedParentData(SceneObject* pObjectToParent, UUID parentID) : ObjectToParent(pObjectToParent), ParentID(parentID) {}
//
//        SceneObject* ObjectToParent;
//        UUID ParentID;
//    };
//
//    class GScene : public Resource
//    {
//    public:
//        SceneObject* GetSceneObject(size_t index);
//
//        SceneObject* FindSceneObject(UUID uuid) const;
// 
//        void DelayedSetParent(SceneObject* pObjectToParent, UUID parentID);
//
//        void HandleDelayedParents();
//
//        SceneObject* InstantiatePrefab(SceneObject* pParent, PrefabData* pPrefab,
//            const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale);
//        SceneObject* InstantiatePrefab(SceneObject* pParent, PrefabData* pPrefab, uint32_t remapSeed,
//            const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale);
//        SceneObject* InstantiatePrefab(SceneObject* pParent, PrefabData* pPrefab, UUIDRemapper& remapper,
//            const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale);
//
//        SceneObject* GetSceneObjectFromEntityID(Utils::ECS::EntityID entity);
//
//    protected:
//        void OnDelayedSetParent(const DelayedParentData& data);
//
//    private:
//        SceneObject* InstantiatePrefabNode(SceneObject* pParent, const PrefabNode& node, UUIDRemapper& remapper);
//
//    protected:
//        std::vector<DelayedParentData> m_DelayedParents;
//    };
//}
