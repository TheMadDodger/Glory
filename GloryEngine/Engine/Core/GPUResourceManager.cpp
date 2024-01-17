#include "GPUResourceManager.h"
#include "EngineProfiler.h"
#include "MaterialInstanceData.h"
#include "Engine.h"
#include "EngineProfiler.h"

#include <algorithm>

namespace Glory
{
	GPUResourceManager::GPUResourceManager(Engine* pEngine): m_pEngine(pEngine) {}

	GPUResourceManager::~GPUResourceManager()
	{
		for (std::map<UUID, Buffer*>::iterator it = m_pBuffers.begin(); it != m_pBuffers.end(); it++)
		{
			delete it->second;
		}

		m_pBuffers.clear();

		for (std::map<UUID, GPUResource*>::iterator it = m_IDResources.begin(); it != m_IDResources.end(); it++)
		{
			delete it->second;
		}

		m_IDResources.clear();
	}

	Buffer* GPUResourceManager::CreateBuffer(uint32_t bufferSize, BufferBindingTarget usageFlag, MemoryUsage memoryFlags, uint32_t bindIndex)
	{
		if (bufferSize == 0) return nullptr;
		m_pEngine->Profiler().BeginSample("GPUResourceManager::CreateBuffer");
		Buffer* pBuffer = CreateBuffer_Internal(bufferSize, usageFlag, memoryFlags, bindIndex);
		pBuffer->m_pOwner = this;
		pBuffer->CreateBuffer();
		m_pBuffers.emplace(pBuffer->m_UUID, pBuffer);
		m_pEngine->Profiler().EndSample();
		return pBuffer;
	}

	Mesh* GPUResourceManager::CreateMesh(MeshData* pMeshData)
	{
		Mesh* pMesh = GetResource<Mesh>(pMeshData);
		if (pMesh) return pMesh;

		m_pEngine->Profiler().BeginSample("GPUResourceManager::CreateMesh");
		uint32_t vertexBufferSize = pMeshData->VertexCount() * pMeshData->VertexSize();
		uint32_t indexBufferSize = pMeshData->IndexCount() * sizeof(uint32_t);
		Buffer* pVertexBuffer = CreateVertexBuffer(vertexBufferSize);
		Buffer* pIndexBuffer = CreateIndexBuffer(indexBufferSize);
		pVertexBuffer->m_pOwner = this;
		pIndexBuffer->m_pOwner = this;
		pVertexBuffer->Assign(pMeshData->Vertices());
		if (pIndexBuffer) pIndexBuffer->Assign(pMeshData->Indices());
		pMesh = CreateMesh_Internal(pMeshData);
		pMesh->m_pOwner = this;
		pMesh->m_UUID = pMeshData->GetGPUUUID();
		pMesh->SetBuffers(pVertexBuffer, pIndexBuffer);
		pMesh->CreateBindingAndAttributeData();
		m_IDResources[pMeshData->GetGPUUUID()] = pMesh;
		m_pEngine->Profiler().EndSample();
		return pMesh;
	}

	Mesh* GPUResourceManager::CreateMeshNoIndexBuffer(MeshData* pMeshData)
	{
		Mesh* pMesh = GetResource<Mesh>(pMeshData);
		if (pMesh) return pMesh;

		m_pEngine->Profiler().BeginSample("GPUResourceManager::CreateMeshNoIndexBuffer");
		pMesh = CreateMesh_Internal(pMeshData);
		pMesh->m_pOwner = this;
		pMesh->m_UUID = pMeshData->GetGPUUUID();
		pMesh->Bind();
		uint32_t vertexBufferSize = pMeshData->VertexCount() * pMeshData->VertexSize();
		Buffer* pVertexBuffer = CreateVertexBuffer(vertexBufferSize);
		pVertexBuffer->m_pOwner = this;
		pVertexBuffer->Bind();
		pVertexBuffer->Assign(pMeshData->Vertices());
		pMesh->SetBuffers(pVertexBuffer, nullptr);
		pMesh->CreateBindingAndAttributeData();
		m_IDResources[pMeshData->GetGPUUUID()] = pMesh;
		m_pEngine->Profiler().EndSample();
		return pMesh;
	}

	Mesh* GPUResourceManager::CreateMesh(uint32_t vertexCount, uint32_t indexCount, InputRate inputRate, uint32_t binding,
		uint32_t stride, PrimitiveType primitiveType, const std::vector<AttributeType>& attributeTypes, Buffer* pVertexBuffer, Buffer* pIndexBuffer)
	{
		m_pEngine->Profiler().BeginSample("GPUResourceManager::CreateMeshNoIndexBuffer");
		Mesh* pMesh = CreateMesh_Internal(vertexCount, indexCount, inputRate, binding, stride, primitiveType, attributeTypes);
		pMesh->m_pOwner = this;
		pMesh->m_UUID = UUID();
		pMesh->Bind();
		pVertexBuffer->Bind();
		if(pIndexBuffer) pIndexBuffer->Bind();
		pMesh->SetBuffers(pVertexBuffer, pIndexBuffer);
		pMesh->CreateBindingAndAttributeData();
		m_IDResources[pMesh->m_UUID] = pMesh;
		m_pEngine->Profiler().EndSample();
		return pMesh;
	}

	Shader* GPUResourceManager::CreateShader(FileData* pShaderFile, const ShaderType& shaderType, const std::string& function)
	{
		Shader* pShader = GetResource<Shader>(pShaderFile);
		if (pShader) return pShader;

		m_pEngine->Profiler().BeginSample("GPUResourceManager::CreateShader");
		pShader = CreateShader_Internal(pShaderFile, shaderType, function);
		pShader->m_pOwner = this;
		pShader->m_UUID = pShaderFile->GetGPUUUID();
		pShader->Initialize();
		m_IDResources[pShaderFile->GetGPUUUID()] = pShader;
		m_pEngine->Profiler().EndSample();
		return pShader;
	}

	Material* GPUResourceManager::CreateMaterial(MaterialData* pMaterialData)
	{
		Material* pMaterial = GetResource<Material>(pMaterialData);
		if (pMaterial && pMaterial->m_Complete)
		{
			pMaterial->m_pMaterialData = pMaterialData;
			return pMaterial;
		}

		m_pEngine->Profiler().BeginSample("GPUResourceManager::CreateMaterial");
		if (pMaterial) pMaterial->Clear();
		else pMaterial = CreateMaterial_Internal(pMaterialData);
		pMaterial->m_pOwner = this;
		pMaterial->m_pMaterialData = pMaterialData;
		pMaterial->m_UUID = pMaterialData->GetGPUUUID();
		for (size_t i = 0; i < pMaterialData->ShaderCount(); i++)
		{
			ShaderSourceData* pShaderSourceData = pMaterialData->GetShaderAt(i);
			FileData* pCompiledShaderSource = pShaderSourceData != nullptr ? pShaderSourceData->GetCompiledShader() : nullptr;
			if (!pCompiledShaderSource) return nullptr;
			const ShaderType& shaderType = pMaterialData->GetShaderTypeAt(i);
			Shader* pShader = CreateShader(pCompiledShaderSource, shaderType, "main");
			pMaterial->AddShader(pShader);
		}

		pMaterial->Initialize();
		pMaterial->m_Complete = true;
		m_IDResources[pMaterialData->GetGPUUUID()] = pMaterial;
		m_pEngine->Profiler().EndSample();
		return pMaterial;
	}

	Texture* GPUResourceManager::CreateTexture(TextureData* pTextureData)
	{
		Texture* pTexture = GetResource<Texture>(pTextureData);
		if (pTexture)
		{
			if (pTexture->IsDirty())
			{
				pTexture->Create(pTextureData);
				pTexture->SetDirty(false);
			}
			return pTexture;
		}

		m_pEngine->Profiler().BeginSample("GPUResourceManager::CreateTexture");
		pTexture = CreateTexture_Internal(pTextureData);
		if (!pTexture) return nullptr;
		pTexture->m_pOwner = this;
		pTexture->m_UUID = pTextureData->GetGPUUUID();
		pTexture->Create(pTextureData);
		m_IDResources[pTextureData->GetGPUUUID()] = pTexture;
		m_pEngine->Profiler().EndSample();
		return pTexture;
	}

	Texture* GPUResourceManager::CreateTexture(uint32_t width, uint32_t height, const PixelFormat& format, const PixelFormat& internalFormat, const ImageType& imageType, uint32_t usageFlags, uint32_t sharingMode, ImageAspect imageAspectFlags, const SamplerSettings& samplerSettings)
	{
		m_pEngine->Profiler().BeginSample("GPUResourceManager::CreateTexture (2)");
		Texture* pTexture = CreateTexture_Internal(width, height, format, internalFormat, imageType, usageFlags, sharingMode, imageAspectFlags, samplerSettings);
		UUID id = UUID();
		pTexture->m_pOwner = this;
		pTexture->m_UUID = id;
		pTexture->Create();

		m_IDResources[id] = pTexture;
		m_pEngine->Profiler().EndSample();
		return pTexture;
	}

	RenderTexture* GPUResourceManager::CreateRenderTexture(const RenderTextureCreateInfo& createInfo)
	{
		m_pEngine->Profiler().BeginSample("GPUResourceManager::CreateRenderTexture");
		RenderTexture* pRenderTexture = CreateRenderTexture_Internal(createInfo);
		UUID id = UUID();
		pRenderTexture->m_pOwner = this;
		pRenderTexture->m_UUID = id;
		pRenderTexture->Initialize();
		m_IDResources[id] = pRenderTexture;
		m_pEngine->Profiler().EndSample();
		return pRenderTexture;
	}

	void GPUResourceManager::Free(GPUResource* pResource)
	{
		ProfileSample s{ &m_pEngine->Profiler(), "GPUResourceManager::Free" };
		if (!pResource) return;
		UUID id = pResource->m_UUID;
		delete pResource;
		auto it = m_IDResources.find(id);
		if (it == m_IDResources.end()) return;
		m_IDResources.erase(it);
	}
	
	void GPUResourceManager::Free(Buffer* pBuffer)
	{
		ProfileSample s{ &m_pEngine->Profiler(), "GPUResourceManager::Free(Buffer)" };
		if (!pBuffer) return;
		UUID id = pBuffer->m_UUID;
		delete pBuffer;
		auto it = m_pBuffers.find(id);
		if (it == m_pBuffers.end()) return;
		m_pBuffers.erase(it);
	}

	bool GPUResourceManager::ResourceExists(Resource* pResource)
	{
		return m_IDResources.find(pResource->GetGPUUUID()) != m_IDResources.end();
	}

	void GPUResourceManager::SetDirty(UUID uuid)
	{
		auto itor = m_IDResources.find(uuid);
		if (itor == m_IDResources.end()) return;
		itor->second->m_IsDirty = true;
	}

	Engine* GPUResourceManager::GetEngine()
	{
		return m_pEngine;
	}
}