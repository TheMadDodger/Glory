#include "PhysicsModule.h"
#include "PhysicsComponents.h"

#include <IEngine.h>
#include <SceneManager.h>

namespace Glory
{
    PhysicsModule::PhysicsModule() {}
    PhysicsModule::~PhysicsModule() {}

    const std::type_info& PhysicsModule::GetBaseModuleType()
    {
        return typeid(PhysicsModule);
    }

    void PhysicsModule::RegisterTypes()
    {
        Reflect::SetReflectInstance(&m_pEngine->Reflection());
        Reflect::RegisterEnum<BodyType>();
        Reflect::RegisterEnum<MotionQuality>();
        Reflect::RegisterEnum<AllowedDOFFlag>();
        Reflect::RegisterType<PhysicsSimulationSettings>();
    }

    void PhysicsModule::Initialize()
    {
    }
}
