#include "Material.h"
#include "Game.h"

namespace Glory
{
	Material::Material(MaterialData* pMaterialData) : m_pMaterialData(pMaterialData)
	{
	}

	Material::~Material()
	{
	}

	void Material::SetProperties()
	{
		m_pMaterialData->CopyProperties(m_FrameProperties);

		if (m_pMaterialData == nullptr) return;
		for (size_t i = 0; i < m_FrameProperties.size(); i++)
		{
			MaterialPropertyData* pPropertyData = &m_FrameProperties[i];
			SetProperty(pPropertyData);
		}
	}

	void Material::AddShader(Shader* pShader)
	{
		m_pShaders.push_back(pShader);
	}

	void Material::SetUBO(UniformBufferObjectTest ubo)
	{
		m_UBO = ubo;
	}

	void Material::SetProperty(MaterialPropertyData* pProperty)
	{
		const std::type_index& type = pProperty->Type();

		if (type == typeid(float))
		{
			float value = std::any_cast<float>(pProperty->Data());
			SetFloat(pProperty->Name(), value);
		}
		else if (type == typeid(ImageData*))
		{
			ImageData* pImage = std::any_cast<ImageData*>(pProperty->Data());
			Texture* pTexture = Game::GetGame().GetEngine()->GetGraphicsModule()->GetResourceManager()->CreateTexture(pImage);
			SetTexture(pProperty->Name(), pTexture);
		}
	}
}
