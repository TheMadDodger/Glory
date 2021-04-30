#pragma once
#include "Buffer.h"
#include <vector>

namespace Glory
{
	class GraphicsMemoryManager
	{
	public:


	private:
		GraphicsMemoryManager();
		virtual ~GraphicsMemoryManager();

	private:
		friend class GraphicsModule;
		//std::vector<GPUMemory*> m_pMemory;
	};
}