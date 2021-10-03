#pragma once
#include "UUID.h"
#include "GPUResource.h"
#include "Buffer.h"
#include <map>
#include <vector>

namespace Glory
{
	class GPUResourceManager
	{
	public: // Module functions
		Buffer* CreateVertexBuffer(uint32_t bufferSize);

	protected: // Internal functions
		virtual Buffer* CreateVertexBuffer_Internal(uint32_t bufferSize) = 0;

	private:
		std::vector<GPUResource*> m_pResources;
		std::map<UUID, GPUResource*> m_IDResources;
	};
}
