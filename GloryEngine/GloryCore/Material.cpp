#include "Material.h"
#include "Game.h"
#include "AnyConverter.h"

namespace Glory
{
	Material::Material(MaterialData* pMaterialData)
		: m_pMaterialData(pMaterialData), m_pPropertiesBuffer(nullptr), m_Complete(false), m_UBO()
	{
	}

	Material::~Material()
	{
	}

	void Material::SetProperties()
	{
		if (m_pMaterialData == nullptr) return;
		std::vector<char>& propertyBuffer = m_pMaterialData->GetBufferReference();
		if (propertyBuffer.empty()) return;
		if (m_pPropertiesBuffer == nullptr) m_pPropertiesBuffer = CreatePropertiesBuffer(propertyBuffer.size());
		m_pPropertiesBuffer->Assign((const void*)propertyBuffer.data());
		m_pPropertiesBuffer->Bind();
	}

	void Material::AddShader(Shader* pShader)
	{
		m_pShaders.push_back(pShader);
	}

	void Material::Clear()
	{
		m_pShaders.clear();
	}

	void Material::SetUBO(UniformBufferObjectTest ubo)
	{
		m_UBO = ubo;
	}
}
