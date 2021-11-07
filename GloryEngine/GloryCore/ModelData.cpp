#include "ModelData.h"

namespace Glory
{
	ModelData::ModelData() : m_pMeshes(std::vector<MeshData*>())
	{
		APPEND_TYPE(ModelData);
	}

	ModelData::~ModelData()
	{
	}

	void ModelData::AddMesh(MeshData* pMesh)
	{
		m_pMeshes.push_back(pMesh);
	}

	size_t ModelData::GetMeshCount() const
	{
		return m_pMeshes.size();
	}

	MeshData* ModelData::GetMesh(size_t index) const
	{
		index = index % m_pMeshes.size();
		return m_pMeshes[index];
	}
}
