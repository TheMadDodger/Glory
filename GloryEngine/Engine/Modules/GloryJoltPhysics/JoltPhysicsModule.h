#pragma once
#include <PhysicsModule.h>
#include <Jolt/Jolt.h>

namespace Glory
{
    class JoltPhysicsModule : public PhysicsModule
    {
	public:
		GLORY_API JoltPhysicsModule();
		GLORY_API virtual ~JoltPhysicsModule();

		GLORY_MODULE_VERSION_H;

	private:
		virtual void Initialize() override;
		virtual void PostInitialize() override;
		virtual void Cleanup() override;
    };
}
