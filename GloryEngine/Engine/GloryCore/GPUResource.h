#pragma once
#include "UUID.h"

namespace Glory
{
	class GPUResource
	{
	public:
		GPUResource();
		virtual ~GPUResource();

	private:
		friend class GPUResourceManager;
		UUID m_UUID;
	};
}