#pragma once
#include "UUID.h"

namespace Glory
{
	class Engine;
	class MaterialData;

	class MaterialManager
	{
	public:
		MaterialManager(Engine* pEngine);
		virtual ~MaterialManager() = default;

		virtual MaterialData* GetMaterial(UUID materialID) const = 0;

	protected:
		Engine* m_pEngine;
	};
}
