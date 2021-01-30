#include "GraphicsModule.h"

namespace Glory
{
	GraphicsModule::GraphicsModule()
	{
	}

	GraphicsModule::~GraphicsModule()
	{
	}

	Buffer* GraphicsModule::CreateVertexBuffer(uint32_t bufferSize)
	{
		Buffer* pVertexBuffer = CreateVertexBuffer_Internal(bufferSize);
		// Store it somewhere
		return pVertexBuffer;
	}
}