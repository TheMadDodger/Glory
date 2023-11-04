#pragma once
#include "Object.h"
#include "Layer.h"
#include "Entity.h"

namespace Glory
{
    class GScene;

    class SceneObject : public Object
    {
    public:
        SceneObject();
        SceneObject(Entity entity);
        SceneObject(Entity entity, const std::string& name);
        SceneObject(Entity entity, const std::string& name, UUID uuid);
        virtual ~SceneObject();

        SceneObject* GetParent();

        size_t ChildCount();
        SceneObject* GetChild(size_t index);
        void SetSiblingIndex(size_t index);
        void SetBeforeObject(SceneObject* pObject);
        void SetAfterObject(SceneObject* pObject);
        size_t GetSiblingIndex();

        void SetScene(GScene* pScene);
        GScene* GetScene() const;

        bool IsActiveSelf() const;
        bool IsActiveInHierarchy() const;
        void SetActive(bool active);
        void SetHierarchyActive();

        Entity GetEntityHandle();

    public:
        void SetParent(SceneObject* pParent);

    protected:
        void OnSetParent(SceneObject* pParent);
        void DestroyOwnChildren();

    private:
        friend class GScene;
        GScene* m_pScene;
        std::vector<SceneObject*> m_pChildren;
        Entity m_Entity;
        SceneObject* m_pParent;
    };
}
