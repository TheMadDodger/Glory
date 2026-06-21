#pragma once
#include <Module.h>

namespace Glory
{
	/** @brief Base class for audio backend and mixing modules */
    class PhysicsModule : public Module
    {
	public:
		/** @brief Constructor */
		PhysicsModule();
		/** @brief Destructor */
		virtual ~PhysicsModule();

		/** @brief PhysicsModule type */
		const std::type_info& GetBaseModuleType() override;
		
	protected:
		virtual void RegisterTypes() override;
		virtual void Initialize() override;
		virtual void Cleanup() = 0;
    };
}
