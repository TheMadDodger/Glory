#include "GraphicsDevice.h"
#include "Module.h"
#include "Engine.h"
#include "Debug.h"
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
	GraphicsDevice::GraphicsDevice(Module* pModule): m_pModule(pModule), m_APIFeatures(APIFeatures::None)
	{
	}

	GraphicsDevice::~GraphicsDevice()
	{
		m_pModule = nullptr;
	}

	void GraphicsDevice::AddBindingIndex(std::string&& name, uint32_t index)
	{
		m_BindingIndices.emplace(std::move(name), index);
	}

	uint32_t GraphicsDevice::BindingIndex(const std::string& name) const
	{
		auto iter = m_BindingIndices.find(name);
		if (iter == m_BindingIndices.end())
		{
			m_pModule->GetEngine()->GetDebug().LogWarning("GraphicsDevice::BindingIndex: No binding index was set for " + name);
			return 0;
		}
		return iter->second;
	}

	PipelineHandle GraphicsDevice::AcquireCachedPipeline(RenderPassHandle renderPass, PipelineData* pPipeline,
		std::vector<DescriptorSetHandle>&& descriptorSets, size_t stride, const std::vector<AttributeType>& attributeTypes)
	{
		auto iter = m_PipelineHandles.find(pPipeline->GetGPUUUID());
		if (iter == m_PipelineHandles.end())
		{
			PipelineHandle newPipeline = CreatePipeline(renderPass, pPipeline, std::move(descriptorSets), stride, attributeTypes);
			iter = m_PipelineHandles.emplace(pPipeline->GetGPUUUID(), newPipeline).first;
		}

		PipelineHandle pipeline = iter->second;

		/* @todo: Update and/or move pipeline if needed */

		return pipeline;
	}

	MeshHandle GraphicsDevice::AcquireCachedMesh(MeshData* pMesh)
	{
		auto iter = m_MeshHandles.find(pMesh->GetGPUUUID());
		if (iter == m_MeshHandles.end())
		{
			MeshHandle newMesh = CreateMesh(pMesh);
			iter = m_MeshHandles.emplace(pMesh->GetGPUUUID(), newMesh).first;
		}

		MeshHandle mesh = iter->second;

		/* @todo: Update mesh if needed */

		return mesh;
	}

	MeshHandle GraphicsDevice::CreateMesh(MeshData* pMeshData)
	{
		std::vector<BufferHandle> buffers(2);
		buffers[0] = CreateBuffer("VertexBuffer", pMeshData->VertexCount()*pMeshData->VertexSize(), BufferType::BT_Vertex);
		buffers[1] = CreateBuffer("IndexBuffer", pMeshData->IndexCount()*sizeof(uint32_t), BufferType::BT_Index);
		AssignBuffer(buffers[0], pMeshData->Vertices(), pMeshData->VertexCount()*pMeshData->VertexSize());
		AssignBuffer(buffers[1], pMeshData->Indices(), pMeshData->IndexCount()*sizeof(uint32_t));
		return CreateMesh(std::move(buffers), pMeshData->VertexCount(), pMeshData->IndexCount(),
			pMeshData->VertexSize(), PrimitiveType::PT_Triangles, pMeshData->AttributeTypesVector());
	}

	Debug& GraphicsDevice::Debug()
	{
		return m_pModule->GetEngine()->GetDebug();
	}

	bool GraphicsDevice::IsSupported(const APIFeatures& features) const
	{
		return m_APIFeatures.HasFlag(features);
	}
}
