#pragma once
#include "MaterialData.h"

namespace Glory
{
	class MaterialManager;

	class MaterialInstanceData : public MaterialData
	{
	public:
		MaterialInstanceData();
		MaterialInstanceData(UUID baseMaterial);
		virtual ~MaterialInstanceData();

	public:
		virtual PipelineData* GetPipeline(const MaterialManager& materialManager, const PipelineManager& pipelineManager) const override;
		virtual UUID GetPipelineID(const MaterialManager& materialManager) const override;

		UUID BaseMaterialID() const;
		MaterialData* GetBaseMaterial(const MaterialManager& manager) const;

		virtual UUID GetGPUUUID() const;

		void CopyOverrideStates(std::vector<bool>& destination);
		void PasteOverrideStates(std::vector<bool>& destination);

		virtual size_t PropertyInfoCount(const MaterialManager& manager) const override;
		virtual MaterialPropertyInfo* GetPropertyInfoAt(const MaterialManager& manager, size_t index) override;
		virtual size_t GetCurrentBufferOffset(const MaterialManager& manager) const override;
		virtual std::vector<char>& GetBufferReference(const MaterialManager& manager) override;
		virtual bool GetPropertyInfoIndex(const MaterialManager& manager, const std::string& name, size_t& index) const override;
		virtual AssetReference<TextureData>* GetResourceUUIDPointer(MaterialManager& manager, size_t index) override;
		virtual size_t GetPropertyIndexFromResourceIndex(MaterialManager& manager, size_t index) const override;
		virtual size_t GetResourcePropertyCount(MaterialManager& manager) const override;
		virtual MaterialPropertyInfo* GetResourcePropertyInfo(MaterialManager& manager, size_t index) override;

		void Resize(const MaterialManager& manager, MaterialData* pBase);

		bool IsPropertyOverriden(size_t index) const;
		virtual void EnableProperty(size_t index) override;
		void DisableProperty(size_t index);

		void Serialize(BinaryStream& container) const override;
		void Deserialize(BinaryStream& container) override;

	private:
		virtual std::vector<char>& GetPropertyBuffer(MaterialManager& manager, size_t index) override;

	private:
		UUID m_BaseMaterial;
		std::vector<bool> m_PropertyOverridesEnable;
	};
}
