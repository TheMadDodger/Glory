#include "GraphicsDevice.h"
#include "Module.h"
#include "Engine.h"
#include "Debug.h"
#include "EngineProfiler.h"

#include "MaterialData.h"
#include "PipelineData.h"
#include "MeshData.h"
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

	void GraphicsDevice::DrawQuad(CommandBufferHandle commandBuffer)
	{
		DrawMesh(commandBuffer, m_ScreenMesh);
	}

	PipelineHandle GraphicsDevice::AcquireCachedPipeline(RenderPassHandle renderPass, PipelineData* pPipeline,
		std::vector<DescriptorSetLayoutHandle>&& descriptorSets, size_t stride, const std::vector<AttributeType>& attributeTypes)
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

	TextureHandle GraphicsDevice::AcquireCachedTexture(TextureData* pTexture)
	{
		auto iter = m_TextureHandles.find(pTexture->GetGPUUUID());
		if (iter == m_TextureHandles.end())
		{
			TextureHandle newTexture = CreateTexture(pTexture);
			iter = m_TextureHandles.emplace(pTexture->GetGPUUUID(), newTexture).first;
		}
		return iter->second;
	}

	bool GraphicsDevice::CachedTextureExists(TextureData* pTexture)
	{
		auto iter = m_TextureHandles.find(pTexture->GetGPUUUID());
		return iter != m_TextureHandles.end();
	}

	MeshHandle GraphicsDevice::CreateMesh(MeshData* pMeshData)
	{
		std::vector<BufferHandle> buffers(2);
		buffers[0] = CreateBuffer(pMeshData->VertexCount()*pMeshData->VertexSize(), BufferType::BT_Vertex, BF_None);
		buffers[1] = CreateBuffer(pMeshData->IndexCount()*sizeof(uint32_t), BufferType::BT_Index, BF_None);
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

	void GraphicsDevice::Initialize()
	{
		static const float vertices[] = {
			-1.0f, -1.0f, 0.0f,
			 1.0f, -1.0f, 0.0f,
			-1.0f,  1.0f, 0.0f,
			-1.0f,  1.0f, 0.0f,
			 1.0f, -1.0f, 0.0f,
			 1.0f,  1.0f, 0.0f,
		};
		BufferHandle buffer = CreateBuffer(sizeof(vertices), BufferType::BT_Vertex, BF_None);
		AssignBuffer(buffer, vertices, sizeof(vertices));
		m_ScreenMesh = CreateMesh({ buffer }, 6, 0, sizeof(glm::vec3), PrimitiveType::PT_Triangles, { AttributeType::Float3 });
	}

	CommandBufferHandle GraphicsDevice::Begin()
	{
		CommandBufferHandle commandBuffer = CreateCommandBuffer();
		Begin(commandBuffer);
		return commandBuffer;
	}
}
