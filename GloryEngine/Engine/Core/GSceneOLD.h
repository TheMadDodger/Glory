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
// 
//        void DelayedSetParent(SceneObject* pObjectToParent, UUID parentID);
//
//        void HandleDelayedParents();
//
//    protected:
//        void OnDelayedSetParent(const DelayedParentData& data);
//
//
//    protected:
//        std::vector<DelayedParentData> m_DelayedParents;
//    };
//}
