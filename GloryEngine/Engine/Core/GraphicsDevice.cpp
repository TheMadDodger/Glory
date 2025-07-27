#include "GraphicsDevice.h"
#include "Module.h"
#include "Engine.h"
#include "EngineProfiler.h"

#include "Buffer.h"
#include "Mesh.h"
#include "Shader.h"
#include "Material.h"
#include "Pipeline.h"
#include "RenderTexture.h"

#include "MaterialData.h"
#include "PipelineData.h"
#include "CubemapData.h"

#define TEMPLATE_GETTER(type) template<> \
type* GraphicsDevice::GetResource<type>(UUID id)\
{\
	return Get##type(id);\
}

#define TEMPLATE_FREE(type) template<> \
void GraphicsDevice::Free<type##Handle>(type##Handle& handle)\
{\
	Free##type(handle);\
	handle = NULL;\
}

namespace Glory
{
	GraphicsDevice::GraphicsDevice(Module* pModule): m_pModule(pModule)
	{
	}

	GraphicsDevice::~GraphicsDevice()
	{
		m_pModule = nullptr;
	}

	MeshHandle GraphicsDevice::CreateMesh(MeshData* pMeshData)
	{
		std::vector<BufferHandle> buffers(2);
		buffers[0] = CreateBuffer(pMeshData->VertexCount()*pMeshData->VertexSize(), BufferType::BT_Vertex);
		buffers[1] = CreateBuffer(pMeshData->IndexCount()*sizeof(uint32_t), BufferType::BT_Index);
		AssignBuffer(buffers[0], pMeshData->Vertices(), pMeshData->VertexCount()*pMeshData->VertexSize());
		AssignBuffer(buffers[1], pMeshData->Indices(), pMeshData->IndexCount()*sizeof(uint32_t));
		return CreateMesh(std::move(buffers), pMeshData->VertexCount(), pMeshData->IndexCount(),
			pMeshData->VertexSize(), PrimitiveType::PT_Triangles, pMeshData->AttributeTypesVector());
	}

	Debug& GraphicsDevice::Debug()
	{
		return m_pModule->GetEngine()->GetDebug();
	}
}
