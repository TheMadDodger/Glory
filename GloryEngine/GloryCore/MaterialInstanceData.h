#pragma once
#include "MaterialData.h"

namespace Glory
{
	class MaterialInstanceData : public MaterialData
	{
	public:
		MaterialInstanceData();
		MaterialInstanceData(MaterialData* pBaseMaterial);
		virtual ~MaterialInstanceData();

	public:
        virtual size_t ShaderCount() const override;
        virtual ShaderSourceData* GetShaderAt(size_t index) const override;;
        virtual const ShaderType& GetShaderTypeAt(size_t index) const override;

		MaterialData* GetBaseMaterial() const;
		void SetBaseMaterial(MaterialData* pMaterial);

		virtual const UUID& GetGPUUUID() const;

		void CopyOverrideStates(std::vector<bool>& destination);
		void PasteOverrideStates(std::vector<bool>& destination);

		virtual size_t PropertyInfoCount() const override;
		virtual MaterialPropertyInfo* GetPropertyInfoAt(size_t index) override;
		virtual size_t GetCurrentBufferOffset() const override;
		virtual std::vector<char>& GetBufferReference() override;
		virtual bool GetPropertyInfoIndex(const std::string& name, size_t& index) const override;
		virtual Resource** GetResourcePointer(size_t index) override;
		virtual size_t GetPropertyIndexFromResourceIndex(size_t index) const override;
		virtual size_t GetResourcePropertyCount() const override;
		virtual MaterialPropertyInfo* GetResourcePropertyInfo(size_t index) override;

		void ReloadProperties();

	private:
		friend class MaterialInstanceLoaderModule;
		MaterialData* m_pBaseMaterial;
		std::vector<bool> m_PropertyOverridesEnable;
	};
}
