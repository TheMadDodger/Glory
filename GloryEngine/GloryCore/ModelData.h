#pragma once
#include "Resource.h"
#include "MeshData.h"
#include <vector>

namespace Glory
{
	class ModelData : public Resource
	{
	public:
		ModelData();
		virtual ~ModelData();

		void AddMesh(MeshData* pMesh);
		size_t GetMeshCount() const;
		MeshData* GetMesh(size_t index) const;

	private:
		std::vector<MeshData*> m_pMeshes;
	};
}