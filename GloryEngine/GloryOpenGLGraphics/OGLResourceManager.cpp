#include "OGLResourceManager.h"
#include "GLBuffer.h"
#include "GLMesh.h"
#include <GL/glew.h>

namespace Glory
{
	OGLResourceManager::OGLResourceManager()
	{
	}

	OGLResourceManager::~OGLResourceManager()
	{
	}

	Mesh* OGLResourceManager::CreateMesh_Internal(MeshData* pMeshData)
	{
		const AttributeType* attributeTypes = pMeshData->AttributeTypes();
		std::vector<AttributeType> attributes = std::vector<AttributeType>(pMeshData->AttributeCount());
		for (size_t i = 0; i < attributes.size(); i++)
			attributes[i] = attributeTypes[i];

		return new GLMesh(pMeshData->VertexCount(), pMeshData->IndexCount(), InputRate::Vertex, 0, pMeshData->VertexSize(), attributes);
	}

	Buffer* OGLResourceManager::CreateVertexBuffer(uint32_t bufferSize)
	{
		return CreateBuffer(bufferSize, GL_ARRAY_BUFFER, GL_STATIC_DRAW);
	}

	Buffer* OGLResourceManager::CreateIndexBuffer(uint32_t bufferSize)
	{
		return CreateBuffer(bufferSize, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW);
	}

	Buffer* OGLResourceManager::CreateBuffer_Internal(uint32_t bufferSize, uint32_t usageFlag, uint32_t memoryFlags)
	{
		return new GLBuffer(bufferSize, usageFlag, memoryFlags);
	}
}