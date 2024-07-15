#include "SoundMaterialData.h"

namespace Glory
{
	SoundMaterialData::SoundMaterialData(): m_Material()
	{
		APPEND_TYPE(SoundMaterialData);
	}

	SoundMaterialData::SoundMaterialData(SoundMaterial&& material): m_Material(std::move(material))
	{
		APPEND_TYPE(SoundMaterialData);
	}

	SoundMaterialData::~SoundMaterialData()
	{
	}

	SoundMaterial& SoundMaterialData::Material()
	{
		return m_Material;
	}
	
	const SoundMaterial& SoundMaterialData::Material() const
	{
		return m_Material;
	}
}
