//#pragma once
//#include "Object.h"
//#include "Layer.h"
//#include "Entity.h"
//
//namespace Glory
//{
//    class GScene;
//
//    class SceneObject : public Object
//    {
//    public:
//        void SetSiblingIndex(size_t index);
//        void SetBeforeObject(SceneObject* pObject);
//        void SetAfterObject(SceneObject* pObject);
//        size_t GetSiblingIndex();
//
//    public:
//        void SetParent(SceneObject* pParent);
//
//    protected:
//        void OnSetParent(SceneObject* pParent);
//        void DestroyOwnChildren();
//
//    private:
//        friend class GScene;
//        GScene* m_pScene;
//        std::vector<SceneObject*> m_pChildren;
//        Entity m_Entity;
//        SceneObject* m_pParent;
//    };
//}
//