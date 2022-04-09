#pragma once
#include "Entity.h"
#include "EntityComponentObject.h"
#include <SceneObject.h>

namespace Glory
{
    class EntitySceneObject : public SceneObject
    {
    public:
        EntitySceneObject(Entity entity);
        EntitySceneObject(Entity entity, const std::string& name);
        EntitySceneObject(Entity entity, const std::string& name, UUID uuid);
        virtual ~EntitySceneObject();

        virtual SceneObject* GetParent() override;

        Entity GetEntityHandle();

    private:
        virtual void Initialize() override;
        virtual void OnSetParent(SceneObject* pParent) override;

    private:
        Entity m_Entity;
        EntitySceneObject* m_pParent;
    };
}
