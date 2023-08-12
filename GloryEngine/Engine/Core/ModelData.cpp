#include "ModelData.h"

namespace Glory
{
	ModelData::ModelData()
	{
		APPEND_TYPE(ModelData);
	}

	ModelData::~ModelData()
	{
	}

	size_t ModelData::GetMeshCount() const
	{
		return m_pSubresources.size();
	}

	MeshData* ModelData::GetMesh(size_t index) const
	{
		index = index % m_pSubresources.size();
		return Subresource<MeshData>(index);
	}

	MeshData* ModelData::GetMesh(std::string_view name) const
	{
		return nullptr;
	}
}
