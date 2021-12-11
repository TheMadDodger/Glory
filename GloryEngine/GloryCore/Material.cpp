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

	void Material::AddShader(Shader* pShader)
	{
		m_pShaders.push_back(pShader);
	}

	void Material::SetUBO(UniformBufferObjectTest ubo)
	{
		m_UBO = ubo;
	}

	void Material::SetTexture(ImageData* pImageData)
	{
		m_pTexture = Game::GetGame().GetEngine()->GetGraphicsModule()->GetResourceManager()->CreateTexture(pImageData);
	}
	void Material::SetTexture(Texture* pTexture)
	{
		m_pTexture = pTexture;
	}
}
