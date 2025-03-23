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

	private:
		virtual void References(Engine* pEngine, std::vector<UUID>& references) const override {}
	};
}