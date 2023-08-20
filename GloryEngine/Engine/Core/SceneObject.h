#pragma once
#include "Object.h"
#include "Layer.h"

namespace Glory
{
    class GScene;

    class SceneObject : public Object
    {
    public:
        SceneObject();
        SceneObject(const std::string& name);
        SceneObject(const std::string& name, UUID uuid);
        virtual ~SceneObject();

        virtual SceneObject* GetParent() { throw new std::exception("Not implemented"); }

        size_t ChildCount();
        SceneObject* GetChild(size_t index);
        void SetSiblingIndex(size_t index);
        void SetBeforeObject(SceneObject* pObject);
        void SetAfterObject(SceneObject* pObject);
        size_t GetSiblingIndex();

        void SetScene(GScene* pScene);
        GScene* GetScene() const;

        virtual bool IsActiveSelf() const { throw new std::exception("Not implemented"); }
        virtual bool IsActiveInHierarchy() const { throw new std::exception("Not implemented"); }
        virtual void SetActive(bool active) { throw new std::exception("Not implemented"); }
        virtual void SetHierarchyActive() { throw new std::exception("Not implemented"); }

    public:
        void SetParent(SceneObject* pParent);

    protected:
        virtual void Initialize() { throw new std::exception("Not implemented"); }
        virtual void OnSetParent(SceneObject* pParent) { throw new std::exception("Not implemented"); }
        void DestroyOwnChildren();

    private:
        friend class GScene;
        GScene* m_pScene;
        std::vector<SceneObject*> m_pChildren;
    };
}
