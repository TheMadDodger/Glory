#include "Material.h"
#include "Engine.h"
#include "AssetManager.h"
#include "GraphicsModule.h"

namespace Glory
{
	Buffer* Material::m_pMVPBuffer = nullptr;

	Material::Material(MaterialData* pMaterialData)
		: m_pMaterialData(pMaterialData), m_pPropertiesBuffer(nullptr),
		m_pHasTexturesBuffer(nullptr), m_Complete(false),
		m_pPipeline(nullptr), m_TextureSetBits(0)
	{
	}

	Material::~Material()
	{
	}

	void Material::SetProperties(Engine* pEngine)
	{
		if (m_pMaterialData == nullptr) return;

		SetSamplers(pEngine);
		SetPropertiesBuffer(pEngine);
	}

	void Material::SetObjectData(const ObjectData& data)
	{
		if (m_pMVPBuffer == nullptr)
			m_pMVPBuffer = CreateMVPBuffer();

		m_pMVPBuffer->Assign((const void*)&data);
		m_pMVPBuffer->BindForDraw();
	}

	void Material::SetSamplers(Engine* pEngine)
	{
		m_TextureSetBits = 0;
		for (size_t i = 0; i < m_pMaterialData->GetResourcePropertyCount(); i++)
		{
			MaterialPropertyInfo* pPropertyInfo = m_pMaterialData->GetResourcePropertyInfo(i);
			const std::string& shaderName = pPropertyInfo->ShaderName();
			const UUID uuid = m_pMaterialData->GetResourceUUIDPointer(i)->AssetUUID();
			TextureData* pTextureData = pEngine->GetAssetManager().GetOrLoadAsset<TextureData>(uuid);
			Texture* pTexture = pTextureData != nullptr ? pEngine->GetMainModule<GraphicsModule>()->GetResourceManager()->CreateTexture(pTextureData) : nullptr;
			SetTexture(shaderName, pTexture);
			m_TextureSetBits |= pTexture ? 1 << i : 0;
		}
	}

	void Material::SetPropertiesBuffer(Engine* pEngine, uint32_t hasTexturesBinding)
	{
		std::vector<char>& propertyBuffer = m_pMaterialData->GetBufferReference();
		if (!propertyBuffer.empty())
		{
			if (m_pPropertiesBuffer == nullptr) m_pPropertiesBuffer = CreatePropertiesBuffer((uint32_t)propertyBuffer.size());
			m_pPropertiesBuffer->Assign((const void*)propertyBuffer.data());
		}
		if (m_pHasTexturesBuffer == nullptr) m_pHasTexturesBuffer = CreateHasTexturesBuffer(hasTexturesBinding);
		m_pHasTexturesBuffer->Assign((const void*)&m_TextureSetBits);

		if (m_pPropertiesBuffer) m_pPropertiesBuffer->BindForDraw();
		m_pHasTexturesBuffer->BindForDraw();
	}
}
