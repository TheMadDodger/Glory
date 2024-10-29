#pragma once
#include <MaterialManager.h>

namespace Glory
{
	/** @brief Runtime material manager */
	class RuntimeMaterialManager : public MaterialManager
	{
	public:
		/** @brief Constructor */
		RuntimeMaterialManager(Engine* pEngine);
		/** @brief Destructor */
		virtual ~RuntimeMaterialManager() = default;

	private:
		virtual MaterialData* GetMaterial(UUID materialID) const override;
		virtual MaterialInstanceData* CreateRuntimeMaterialInstance(UUID baseMaterial) override;
		virtual void DestroyRuntimeMaterials() override;

	private:
		std::vector<UUID> m_RuntimeMaterials;
	};
} 
