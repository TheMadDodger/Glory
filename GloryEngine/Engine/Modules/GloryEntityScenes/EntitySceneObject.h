#pragma once
#include <SceneObject.h>
#include <Glory.h>
#include "Entity.h"
#include "EntityComponentObject.h"

namespace Glory
{
    class EntitySceneObject : public SceneObject
    {
    public:
        GLORY_API EntitySceneObject(Entity entity);
        GLORY_API EntitySceneObject(Entity entity, const std::string& name);
        GLORY_API EntitySceneObject(Entity entity, const std::string& name, UUID uuid);
        GLORY_API virtual ~EntitySceneObject();

        GLORY_API virtual SceneObject* GetParent() override;

        GLORY_API Entity GetEntityHandle();

        bool IsActiveSelf() const override;
        bool IsActiveInHierarchy() const override;
        void SetActive(bool active) override;
        void SetHierarchyActive(bool active) override;

    private:
        virtual void Initialize() override;
        virtual void OnSetParent(SceneObject* pParent) override;

    private:
        Entity m_Entity;
        EntitySceneObject* m_pParent;
    };
}
