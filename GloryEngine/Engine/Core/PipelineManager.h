#pragma once
#include "UUID.h"

namespace Glory
{
	class Engine;
	class PipelineData;

	class PipelineManager
	{
	public:
		PipelineManager(Engine* pEngine);
		virtual ~PipelineManager() = default;

		virtual PipelineData* GetPipelineData(UUID materialID) const = 0;

	protected:
		Engine* m_pEngine;
	};
}
