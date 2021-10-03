#include "GPUResourceManager.h"

namespace Glory
{
	Buffer* GPUResourceManager::CreateVertexBuffer(uint32_t bufferSize)
	{
		Buffer* pVertexBuffer = CreateVertexBuffer_Internal(bufferSize);
		m_pResources.push_back(pVertexBuffer);
		return pVertexBuffer;
	}
}