#pragma once
#include "GScene.h"

#include <EntityID.h>
#include <EntityRegistry.h>

namespace Glory
{
    class PrefabData;
    class Entity;

    class PrefabData : public GScene
    {
    public:
        PrefabData();
        virtual ~PrefabData() = default;
        static PrefabData* CreateFromEntity(GScene* pScene, Utils::ECS::EntityID entity);

    private:
        static void CopyEntity(PrefabData* pPrefab, GScene* pScene, Utils::ECS::EntityID entity, Utils::ECS::EntityID parent);

        /** @brief Get a vector containing other resources referenced by this resource */
        virtual void References(Engine* pEngine, std::vector<UUID>& references) const override;
    };
}
