#include "PhysicsModule.h"
#include "PhysicsComponents.h"

#include <GloryECS/EntityRegistry.h>
#include <GloryECS/EntityView.h>

#include <Engine.h>
#include <SceneManager.h>

namespace Glory
{
    PhysicsModule::PhysicsModule() {}
    PhysicsModule::~PhysicsModule() {}

    const std::type_info& PhysicsModule::GetBaseModuleType()
    {
        return typeid(PhysicsModule);
    }

    void PhysicsModule::Initialize()
    {
        Reflect::SetReflectInstance(&m_pEngine->Reflection());
        Reflect::RegisterEnum<BodyType>();
        Reflect::RegisterEnum<MotionQuality>();
        Reflect::RegisterEnum<AllowedDOFFlag>();
        Reflect::RegisterType<PhysicsSimulationSettings>();

        m_pEngine->GetSceneManager()->RegisterComponent<PhysicsBody>();
        m_pEngine->GetSceneManager()->RegisterComponent<CharacterController>();
    }
}
