#include "Material.h"
#include "Engine.h"
#include "Resources.h"
#include "GraphicsModule.h"

namespace Glory
{
	Buffer* Material::m_pMVPBuffer = nullptr;

	Material::Material(MaterialData* pMaterialData)
		: m_pMaterialData(pMaterialData), m_pPropertiesBuffer(nullptr), m_Complete(false)
	{
	}

	Material::~Material()
	{
	}

	void Material::SetProperties(Engine* pEngine)
	{
		if (m_pMaterialData == nullptr) return;

		for (size_t i = 0; i < m_pMaterialData->GetResourcePropertyCount(pEngine->GetMaterialManager()); i++)
		{
			MaterialPropertyInfo* pPropertyInfo = m_pMaterialData->GetResourcePropertyInfo(pEngine->GetMaterialManager(), i);
			const std::string& shaderName = pPropertyInfo->ShaderName();
			const UUID uuid = m_pMaterialData->GetResourceUUIDPointer(pEngine->GetMaterialManager(), i)->AssetUUID();
			TextureData* pTextureData = pEngine->GetResources().Manager<TextureData>()->Get(uuid);
			Texture* pTexture = pTextureData != nullptr ? pEngine->GetMainModule<GraphicsModule>()->GetResourceManager()->CreateTexture(pTextureData) : nullptr;
			SetTexture(shaderName, pTexture);
		}

		std::vector<char>& propertyBuffer = m_pMaterialData->GetBufferReference(pEngine->GetMaterialManager());
		if (propertyBuffer.empty()) return;
		if (m_pPropertiesBuffer == nullptr) m_pPropertiesBuffer = CreatePropertiesBuffer((uint32_t)propertyBuffer.size());
		m_pPropertiesBuffer->Assign((const void*)propertyBuffer.data());
		m_pPropertiesBuffer->Bind();
	}

	void Material::SetObjectData(const ObjectData& data)
	{
		if (m_pMVPBuffer == nullptr)
			m_pMVPBuffer = CreateMVPBuffer();

		m_pMVPBuffer->Assign((const void*)&data);
		m_pMVPBuffer->Bind();
	}

	void Material::AddShader(Shader* pShader)
	{
		m_pShaders.push_back(pShader);
	}

	void Material::Clear()
	{
		m_pShaders.clear();
	}
}
