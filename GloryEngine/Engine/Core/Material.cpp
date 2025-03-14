#include "Material.h"
#include "Engine.h"
#include "AssetManager.h"
#include "GraphicsModule.h"

namespace Glory
{
	Buffer* Material::m_pMVPBuffer = nullptr;

	Material::Material(MaterialData* pMaterialData)
		: m_pMaterialData(pMaterialData), m_pPropertiesBuffer(nullptr), m_pHasTexturesBuffer(nullptr), m_Complete(false), m_pPipeline(nullptr)
	{
	}

	Material::~Material()
	{
	}

	void Material::SetProperties(Engine* pEngine)
	{
		if (m_pMaterialData == nullptr) return;

		uint64_t textureBits = 0;

		for (size_t i = 0; i < m_pMaterialData->GetResourcePropertyCount(pEngine->GetMaterialManager()); i++)
		{
			MaterialPropertyInfo* pPropertyInfo = m_pMaterialData->GetResourcePropertyInfo(pEngine->GetMaterialManager(), i);
			const std::string& shaderName = pPropertyInfo->ShaderName();
			const UUID uuid = m_pMaterialData->GetResourceUUIDPointer(pEngine->GetMaterialManager(), i)->AssetUUID();
			TextureData* pTextureData = pEngine->GetAssetManager().GetOrLoadAsset<TextureData>(uuid);
			Texture* pTexture = pTextureData != nullptr ? pEngine->GetMainModule<GraphicsModule>()->GetResourceManager()->CreateTexture(pTextureData) : nullptr;
			SetTexture(shaderName, pTexture);
			textureBits |= pTexture ? 1 << i : 0;
		}

		std::vector<char>& propertyBuffer = m_pMaterialData->GetBufferReference(pEngine->GetMaterialManager());
		if (propertyBuffer.empty()) return;
		if (m_pPropertiesBuffer == nullptr) m_pPropertiesBuffer = CreatePropertiesBuffer((uint32_t)propertyBuffer.size());
		if (m_pHasTexturesBuffer == nullptr) m_pHasTexturesBuffer = CreateHasTexturesBuffer();
		m_pPropertiesBuffer->Assign((const void*)propertyBuffer.data());
		m_pHasTexturesBuffer->Assign((const void*)&textureBits);
		m_pPropertiesBuffer->BindForDraw();
		m_pHasTexturesBuffer->BindForDraw();
	}

	void Material::SetObjectData(const ObjectData& data)
	{
		if (m_pMVPBuffer == nullptr)
			m_pMVPBuffer = CreateMVPBuffer();

		m_pMVPBuffer->Assign((const void*)&data);
		m_pMVPBuffer->BindForDraw();
	}
}
