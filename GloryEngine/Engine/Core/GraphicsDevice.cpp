#include "GraphicsDevice.h"
#include "Module.h"
#include "Engine.h"
#include "Debug.h"
#include "EngineProfiler.h"

#include "MaterialData.h"
#include "PipelineData.h"
#include "MeshData.h"
#include "FileData.h"
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
	GraphicsDevice::GraphicsDevice(Module* pModule):
		m_pModule(pModule), m_APIFeatures(APIFeatures::None), m_CurrentDrawCalls(0), m_LastDrawCalls(0),
		m_LastVertices(0), m_CurrentVertices(0), m_LastTriangles(0), m_CurrentTriangles(0)
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

	void GraphicsDevice::DrawUnitCube(CommandBufferHandle commandBuffer)
	{
		DrawMesh(commandBuffer, m_UnitCubeMesh);
	}

	PipelineHandle GraphicsDevice::AcquireCachedPipeline(RenderPassHandle renderPass, PipelineData* pPipeline,
		std::vector<DescriptorSetLayoutHandle>&& descriptorSets, size_t stride,
		const std::vector<AttributeType>& attributeTypes)
	{
		auto iter = m_PipelineHandles.find(pPipeline->GetGPUUUID());
		if (iter == m_PipelineHandles.end())
		{
			PipelineHandle newPipeline = CreatePipeline(renderPass, pPipeline,
				std::move(descriptorSets), stride, attributeTypes);
			m_PipelineHandles.emplace(pPipeline->GetGPUUUID(), newPipeline).first;

			pPipeline->SetDirty(false);
			pPipeline->SettingsDirty() = false;

			return newPipeline;
		}

		PipelineHandle pipeline = iter->second;

		if (pPipeline->IsDirty())
			RecreatePipeline(pipeline, pPipeline);
		else if (pPipeline->SettingsDirty())
			UpdatePipelineSettings(pipeline, pPipeline);

		pPipeline->SetDirty(false);
		pPipeline->SettingsDirty() = false;

		return pipeline;
	}

	PipelineHandle GraphicsDevice::AcquireCachedComputePipeline(PipelineData* pPipeline,
		std::vector<DescriptorSetLayoutHandle>&& descriptorSets)
	{
		auto iter = m_PipelineHandles.find(pPipeline->GetGPUUUID());
		if (iter == m_PipelineHandles.end())
		{
			PipelineHandle newPipeline = CreateComputePipeline(pPipeline, std::move(descriptorSets));
			m_PipelineHandles.emplace(pPipeline->GetGPUUUID(), newPipeline).first;

			pPipeline->SetDirty(false);
			pPipeline->SettingsDirty() = false;

			return newPipeline;
		}

		pPipeline->SetDirty(false);
		pPipeline->SettingsDirty() = false;

		return iter->second;
	}

	MeshHandle GraphicsDevice::AcquireCachedMesh(MeshData* pMesh, MeshUsage usage)
	{
		auto iter = m_MeshHandles.find(pMesh->GetGPUUUID());
		if (iter == m_MeshHandles.end())
		{
			MeshHandle newMesh = CreateMesh(pMesh, usage);
			iter = m_MeshHandles.emplace(pMesh->GetGPUUUID(), newMesh).first;
			pMesh->SetDirty(false);
			return newMesh;
		}

		MeshHandle mesh = iter->second;

		if (pMesh->IsDirty())
		{
			UpdateMesh(mesh, pMesh);
			pMesh->SetDirty(false);
		}

		return mesh;
	}

	TextureHandle GraphicsDevice::AcquireCachedTexture(TextureData* pTexture)
	{
		if (!pTexture) return m_DefaultTexture;

		auto iter = m_TextureHandles.find(pTexture->GetGPUUUID());
		if (iter == m_TextureHandles.end())
		{
			TextureHandle newTexture = CreateTexture(pTexture);
			iter = m_TextureHandles.emplace(pTexture->GetGPUUUID(), newTexture).first;
			pTexture->SetDirty(false);
			return newTexture;
		}

		TextureHandle texture = iter->second;
		ImageData* pImage = pTexture->GetImageData(&m_pModule->GetEngine()->GetAssetManager());

		if (pTexture->IsDirty() || (pImage && pImage->IsDirty()))
		{
			UpdateTexture(texture, pTexture);
			pTexture->SetDirty(false);
			pImage->SetDirty(false);
		}

		return texture;
	}

	TextureHandle GraphicsDevice::AcquireCachedTexture(CubemapData* pCubemap)
	{
		auto iter = m_TextureHandles.find(pCubemap->GetGPUUUID());
		if (iter == m_TextureHandles.end())
		{
			TextureHandle newTexture = CreateTexture(pCubemap);
			iter = m_TextureHandles.emplace(pCubemap->GetGPUUUID(), newTexture).first;
			pCubemap->SetDirty(false);
			return newTexture;
		}
		return iter->second;
	}

	bool GraphicsDevice::CachedTextureExists(TextureData* pTexture)
	{
		auto iter = m_TextureHandles.find(pTexture->GetGPUUUID());
		return iter != m_TextureHandles.end();
	}

	ShaderHandle GraphicsDevice::AcquireCachedShader(const FileData* pShaderFileData, const ShaderType& shaderType, const std::string& function)
	{
		const size_t hash = pShaderFileData->GetMetaData<PipelineShaderMetaData>().m_Hash;
		auto iter = m_ShaderHandles.find(hash);
		if (iter == m_ShaderHandles.end())
		{
			ShaderHandle newShader = CreateShader(pShaderFileData, shaderType, function);
			iter = m_ShaderHandles.emplace(hash, newShader).first;
			return newShader;
		}
		return iter->second;
	}

	void GraphicsDevice::SetCachedTexture(TextureData* pTexture, TextureHandle texture)
	{
		m_TextureHandles[pTexture->GetUUID()] = texture;
	}

	TextureHandle GraphicsDevice::GetCachedTexture(TextureData* pTexture) const
	{
		auto iter = m_TextureHandles.find(pTexture->GetGPUUUID());
		if (iter == m_TextureHandles.end()) return nullptr;
		return iter->second;
	}

	MeshHandle GraphicsDevice::CreateMesh(MeshData* pMeshData, MeshUsage usage)
	{
		BufferFlags bufferFlags = BF_None;
		switch (usage)
		{
		case Glory::MU_Static:
			bufferFlags = BF_CopyDst;
			break;
		case Glory::MU_Dynamic:
			bufferFlags = BF_Write;
			break;
		default:
			break;
		}

		std::vector<BufferHandle> buffers(2);
		buffers[0] = CreateBuffer(pMeshData->VertexCount()*pMeshData->VertexSize(), BufferType::BT_Vertex, BF_CopyDst);
		buffers[1] = CreateBuffer(pMeshData->IndexCount()*sizeof(uint32_t), BufferType::BT_Index, BF_CopyDst);
		AssignBuffer(buffers[0], pMeshData->Vertices(), pMeshData->VertexCount()*pMeshData->VertexSize());
		AssignBuffer(buffers[1], pMeshData->Indices(), pMeshData->IndexCount()*sizeof(uint32_t));
		return CreateMesh(std::move(buffers), pMeshData->VertexCount(), pMeshData->IndexCount(),
			pMeshData->VertexSize(), pMeshData->AttributeTypesVector());
	}

	Debug& GraphicsDevice::Debug()
	{
		return m_pModule->GetEngine()->GetDebug();
	}

	EngineProfiler& GraphicsDevice::Profiler()
	{
		return m_pModule->GetEngine()->Profiler();
	}

	bool GraphicsDevice::IsSupported(const APIFeatures& features) const
	{
		return m_APIFeatures.HasFlag(features);
	}

	void GraphicsDevice::Initialize()
	{
		const bool flipY = GetViewportOrigin() == ViewportOrigin::TopLeft;

		static const float vertices[] = {
			-1.0f, -1.0f*(flipY? -1.0f : 1.0), 0.0f,
			 1.0f, -1.0f*(flipY? -1.0f : 1.0), 0.0f,
			-1.0f,  1.0f*(flipY? -1.0f : 1.0), 0.0f,
			-1.0f,  1.0f*(flipY? -1.0f : 1.0), 0.0f,
			 1.0f, -1.0f*(flipY? -1.0f : 1.0), 0.0f,
			 1.0f,  1.0f*(flipY? -1.0f : 1.0), 0.0f,
		};
		BufferHandle buffer = CreateBuffer(sizeof(vertices), BufferType::BT_Vertex, BF_CopyDst);
		AssignBuffer(buffer, vertices, sizeof(vertices));
		m_ScreenMesh = CreateMesh({ buffer }, 6, 0, sizeof(glm::vec3), { AttributeType::Float3 });

		static const float cubeVertices[] = {
			-1.0f,-1.0f,-1.0f,
			-1.0f,-1.0f, 1.0f,
			-1.0f, 1.0f, 1.0f,
			1.0f, 1.0f,-1.0f,
			-1.0f,-1.0f,-1.0f,
			-1.0f, 1.0f,-1.0f,
			1.0f,-1.0f, 1.0f,
			-1.0f,-1.0f,-1.0f,
			1.0f,-1.0f,-1.0f,
			1.0f, 1.0f,-1.0f,
			1.0f,-1.0f,-1.0f,
			-1.0f,-1.0f,-1.0f,
			-1.0f,-1.0f,-1.0f,
			-1.0f, 1.0f, 1.0f,
			-1.0f, 1.0f,-1.0f,
			1.0f,-1.0f, 1.0f,
			-1.0f,-1.0f, 1.0f,
			-1.0f,-1.0f,-1.0f,
			-1.0f, 1.0f, 1.0f,
			-1.0f,-1.0f, 1.0f,
			1.0f,-1.0f, 1.0f,
			1.0f, 1.0f, 1.0f,
			1.0f,-1.0f,-1.0f,
			1.0f, 1.0f,-1.0f,
			1.0f,-1.0f,-1.0f,
			1.0f, 1.0f, 1.0f,
			1.0f,-1.0f, 1.0f,
			1.0f, 1.0f, 1.0f,
			1.0f, 1.0f,-1.0f,
			-1.0f, 1.0f,-1.0f,
			1.0f, 1.0f, 1.0f,
			-1.0f, 1.0f,-1.0f,
			-1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, 1.0f,
			-1.0f, 1.0f, 1.0f,
			1.0f,-1.0f, 1.0f
		};
		buffer = CreateBuffer(sizeof(cubeVertices), BufferType::BT_Vertex, BF_CopyDst);
		AssignBuffer(buffer, cubeVertices, sizeof(cubeVertices));
		m_UnitCubeMesh = CreateMesh({ buffer }, 36, 0, sizeof(glm::vec3), { AttributeType::Float3 });

		TextureCreateInfo defaultTextureInfo;
		defaultTextureInfo.m_Width = 1;
		defaultTextureInfo.m_Height = 1;
		defaultTextureInfo.m_ImageAspectFlags = IA_Color;
		defaultTextureInfo.m_ImageType = ImageType::IT_2D;
		defaultTextureInfo.m_InternalFormat = PixelFormat::PF_R8G8B8A8Srgb;
		defaultTextureInfo.m_PixelFormat = PixelFormat::PF_RGBA;
		defaultTextureInfo.m_Type = DataType::DT_UByte;
		defaultTextureInfo.m_SamplerSettings.MipmapMode = Filter::F_None;
		defaultTextureInfo.m_SamplerSettings.MaxAnisotropy = 1.0f;
		defaultTextureInfo.m_SamplerSettings.MinFilter = Filter::F_Nearest;
		defaultTextureInfo.m_SamplerSettings.MagFilter = Filter::F_Nearest;

		static constexpr uint8_t defaultTexturePixels[] = {
			255, 0, 255, 255
		};
		m_DefaultTexture = CreateTexture(defaultTextureInfo, defaultTexturePixels, sizeof(defaultTexturePixels));

		OnInitialize();
	}

	void GraphicsDevice::BeginFrame()
	{
		m_CurrentDrawCalls = 0;
		m_CurrentVertices = 0;
		m_CurrentTriangles = 0;
	}

	void GraphicsDevice::EndFrame()
	{
		m_LastDrawCalls = m_CurrentDrawCalls;
		m_LastVertices = m_CurrentVertices;
		m_LastTriangles = m_CurrentTriangles;
	}

	int GraphicsDevice::GetLastDrawCalls() const
	{
		return m_LastDrawCalls;
	}

	int GraphicsDevice::GetLastVertexCount() const
	{
		return m_LastVertices;
	}

	int GraphicsDevice::GetLastTriangleCount() const
	{
		return m_LastTriangles;
	}

	CommandBufferHandle GraphicsDevice::Begin()
	{
		CommandBufferHandle commandBuffer = CreateCommandBuffer();
		Begin(commandBuffer);
		return commandBuffer;
	}
}
