#pragma once
#include "Components.h"

namespace GloryECS
{
    class EntityRegistry;
}

namespace Glory
{
    class CameraSystem
    {
    private:
        CameraSystem();
        virtual ~CameraSystem();

    public:
        static void OnComponentAdded(GloryECS::EntityRegistry* pRegistry, EntityID entity, CameraComponent& pComponent);
        static void OnComponentRemoved(GloryECS::EntityRegistry* pRegistry, EntityID entity, CameraComponent& pComponent);
        static void OnUpdate(GloryECS::EntityRegistry* pRegistry, EntityID entity, CameraComponent& pComponent);
        static void OnDraw(GloryECS::EntityRegistry* pRegistry, EntityID entity, CameraComponent& pComponent);
        static std::string Name();

        static uint32_t CalcHash(CameraComponent& pComponent);
    };
}
