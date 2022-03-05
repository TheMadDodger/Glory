#include "Material.h"
#include "Game.h"
#include "AnyConverter.h"

namespace Glory
{
	Material::Material(MaterialData* pMaterialData)
		: m_pMaterialData(pMaterialData), m_pPropertiesBuffer(nullptr)
	{
	}

	Material::~Material()
	{
	}

	void Material::SetProperties()
	{
		if (m_pMaterialData == nullptr) return;
		m_pMaterialData->CopyProperties(m_FrameProperties);

		m_PropertyData.clear();
		for (size_t i = 0; i < m_FrameProperties.size(); i++)
		{
			MaterialPropertyData* pPropertyData = &m_FrameProperties[i];
			SetProperty(pPropertyData);
		}

		if (m_PropertyData.empty()) return;
		if (m_pPropertiesBuffer == nullptr) m_pPropertiesBuffer = CreatePropertiesBuffer(m_PropertyData.size());
		m_pPropertiesBuffer->Assign((const void*)m_PropertyData.data());
		m_pPropertiesBuffer->Bind();

		//SetMatrix4("mvp", m_UBO.model);
		//SetMatrix4("view", m_UBO.view);
		//SetMatrix4("proj", m_UBO.proj);
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

		if (type == typeid(ImageData*))
		{
			ImageData* pImage = std::any_cast<ImageData*>(pProperty->Data());
			Texture* pTexture = Game::GetGame().GetEngine()->GetGraphicsModule()->GetResourceManager()->CreateTexture(pImage);
			SetTexture(pProperty->Name(), pTexture);
			return;
		}

		AnyConverter::Convert(pProperty->Data(), m_PropertyData);
	}
}
