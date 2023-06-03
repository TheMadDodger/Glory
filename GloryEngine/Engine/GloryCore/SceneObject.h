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

        virtual SceneObject* GetParent() = 0;

        size_t ChildCount();
        SceneObject* GetChild(size_t index);
        void SetSiblingIndex(size_t index);
        void SetBeforeObject(SceneObject* pObject);
        void SetAfterObject(SceneObject* pObject);
        size_t GetSiblingIndex();

        void SetScene(GScene* pScene);
        GScene* GetScene() const;

        virtual bool IsActiveSelf() const = 0;
        virtual bool IsActiveInHierarchy() const = 0;
        virtual void SetActive(bool active) = 0;
        virtual void SetHierarchyActive(bool active) = 0;

    public:
        void SetParent(SceneObject* pParent);

    protected:
        virtual void Initialize() = 0;
        virtual void OnSetParent(SceneObject* pParent) = 0;
        void DestroyOwnChildren();

    private:
        friend class GScene;
        GScene* m_pScene;
        std::vector<SceneObject*> m_pChildren;
    };
}
