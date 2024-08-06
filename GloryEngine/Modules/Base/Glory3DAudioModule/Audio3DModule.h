#pragma once
#include <Module.h>
#include <Hash.h>

#define FEATURE(name) static constexpr uint32_t name = Hashing::Hash(STRINGIFY(AudioFeature_##name))

namespace Glory
{
    /** @brief Base class for modules processing 3D effects on audio */
    class Audio3DModule : public Module
    {
	public:
		/** @brief Constructor */
		Audio3DModule();
		/** @brief Destructor */
		virtual ~Audio3DModule();

		/** @brief Glory3DAudioModule type */
		const std::type_info& GetBaseModuleType() override;

		/** @brief Check if a feature is supported */
		virtual bool HasFeature(uint32_t feature) const = 0;

		/** @brief 3D Audio feature hashes */
		struct Features
		{
			/** @brief Occlusion */
			FEATURE(Occlusion);
			/** @brief Transmission */
			FEATURE(Transmission);
			/** @brief Absorption */
			FEATURE(Absorption);
			/** @brief Reflection */
			FEATURE(Reflection);
			/** @brief Reflection */
			FEATURE(DirectSimulation);
		};
    };
}
