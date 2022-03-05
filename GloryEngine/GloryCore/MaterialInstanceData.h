#pragma once
#include "MaterialData.h"

namespace Glory
{
	class MaterialInstanceData : public MaterialData
	{
	public:
		MaterialInstanceData(MaterialData* pBaseMaterial);
		virtual ~MaterialInstanceData();

	public:
        virtual size_t ShaderCount() const override;
        virtual FileData* GetShaderAt(size_t index) const override;
        virtual const ShaderType& GetShaderTypeAt(size_t index) const override;

		MaterialData* GetBaseMaterial() const;

		virtual const UUID& GetGPUUUID() const;

		virtual void CopyProperties(std::vector<MaterialPropertyData>& destination) override;
		virtual void PasteProperties(const std::vector<MaterialPropertyData>& destination) override;
		void CopyOverrideStates(std::vector<bool>& destination);
		void PasteOverrideStates(std::vector<bool>& destination);

	private:
		friend class MaterialLoaderModule;
		MaterialData* m_pBaseMaterial;
		std::vector<bool> m_PropertyOverridesEnable;
	};
}
