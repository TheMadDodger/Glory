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
		ModelData(UUID uuid, const std::string& name);
		virtual ~ModelData();

		size_t GetMeshCount() const;
		MeshData* GetMesh(size_t index) const;
		MeshData* GetMesh(std::string_view name) const;
	};
}