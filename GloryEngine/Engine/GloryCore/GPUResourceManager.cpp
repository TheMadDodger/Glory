#include "GPUResourceManager.h"
#include "EngineProfiler.h"
#include <algorithm>
#include "MaterialInstanceData.h"

namespace Glory
{
	GPUResourceManager::GPUResourceManager() {}

	GPUResourceManager::~GPUResourceManager()
	{
		for (std::map<UUID, GPUResource*>::iterator it = m_IDResources.begin(); it != m_IDResources.end(); it++)
		{
			delete it->second;
		}

		std::for_each(m_pBuffers.begin(), m_pBuffers.end(), [](Buffer* pBuffer) { delete pBuffer; });
		m_pBuffers.clear();
	}

	Buffer* GPUResourceManager::CreateBuffer(uint32_t bufferSize, BufferBindingTarget usageFlag, MemoryUsage memoryFlags, uint32_t bindIndex)
	{
		if (bufferSize == 0) return nullptr;
		Profiler::BeginSample("GPUResourceManager::CreateBuffer");
		Buffer* pBuffer = CreateBuffer_Internal(bufferSize, usageFlag, memoryFlags, bindIndex);
		pBuffer->CreateBuffer();
		m_pBuffers.push_back(pBuffer);
		Profiler::EndSample();
		return pBuffer;
	}

	Mesh* GPUResourceManager::CreateMesh(MeshData* pMeshData)
	{
		Mesh* pMesh = GetResource<Mesh>(pMeshData);
		if (pMesh) return pMesh;

		Profiler::BeginSample("GPUResourceManager::CreateMesh");
		uint32_t vertexBufferSize = pMeshData->VertexCount() * pMeshData->VertexSize();
		uint32_t indexBufferSize = pMeshData->IndexCount() * sizeof(uint32_t);
		Buffer* pVertexBuffer = CreateVertexBuffer(vertexBufferSize);
		Buffer* pIndexBuffer = CreateIndexBuffer(indexBufferSize);
		pVertexBuffer->Assign(pMeshData->Vertices());
		if (pIndexBuffer) pIndexBuffer->Assign(pMeshData->Indices());
		pMesh = CreateMesh_Internal(pMeshData);
		pMesh->m_UUID = pMeshData->GetGPUUUID();
		pMesh->SetBuffers(pVertexBuffer, pIndexBuffer);
		pMesh->CreateBindingAndAttributeData();
		m_IDResources[pMeshData->GetGPUUUID()] = pMesh;
		Profiler::EndSample();
		return pMesh;
	}

	Mesh* GPUResourceManager::CreateMeshNoIndexBuffer(MeshData* pMeshData)
	{
		Mesh* pMesh = GetResource<Mesh>(pMeshData);
		if (pMesh) return pMesh;

		Profiler::BeginSample("GPUResourceManager::CreateMeshNoIndexBuffer");
		pMesh = CreateMesh_Internal(pMeshData);
		pMesh->m_UUID = pMeshData->GetGPUUUID();
		pMesh->Bind();
		uint32_t vertexBufferSize = pMeshData->VertexCount() * pMeshData->VertexSize();
		Buffer* pVertexBuffer = CreateVertexBuffer(vertexBufferSize);
		pVertexBuffer->Bind();
		pVertexBuffer->Assign(pMeshData->Vertices());
		pMesh->SetBuffers(pVertexBuffer, nullptr);
		pMesh->CreateBindingAndAttributeData();
		m_IDResources[pMeshData->GetGPUUUID()] = pMesh;
		Profiler::EndSample();
		return pMesh;
	}

	Mesh* GPUResourceManager::CreateMesh(uint32_t vertexCount, uint32_t indexCount, InputRate inputRate, uint32_t binding,
		uint32_t stride, PrimitiveType primitiveType, const std::vector<AttributeType>& attributeTypes, Buffer* pVertexBuffer, Buffer* pIndexBuffer)
	{
		Profiler::BeginSample("GPUResourceManager::CreateMeshNoIndexBuffer");
		Mesh* pMesh = CreateMesh_Internal(vertexCount, indexCount, inputRate, binding, stride, primitiveType, attributeTypes);
		pMesh->m_UUID = UUID();
		pMesh->Bind();
		pVertexBuffer->Bind();
		if(pIndexBuffer) pIndexBuffer->Bind();
		pMesh->SetBuffers(pVertexBuffer, pIndexBuffer);
		pMesh->CreateBindingAndAttributeData();
		m_IDResources[pMesh->m_UUID] = pMesh;
		Profiler::EndSample();
		return pMesh;
	}

	Shader* GPUResourceManager::CreateShader(FileData* pShaderFile, const ShaderType& shaderType, const std::string& function)
	{
		Shader* pShader = GetResource<Shader>(pShaderFile);
		if (pShader) return pShader;

		Profiler::BeginSample("GPUResourceManager::CreateShader");
		pShader = CreateShader_Internal(pShaderFile, shaderType, function);
		pShader->m_UUID = pShaderFile->GetGPUUUID();
		pShader->Initialize();
		m_IDResources[pShaderFile->GetGPUUUID()] = pShader;
		Profiler::EndSample();
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

		Profiler::BeginSample("GPUResourceManager::CreateMaterial");
		if (pMaterial) pMaterial->Clear();
		else pMaterial = CreateMaterial_Internal(pMaterialData);
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
		Profiler::EndSample();
		return pMaterial;
	}

	Texture* GPUResourceManager::CreateTexture(ImageData* pImageData)
	{
		Texture* pTexture = GetResource<Texture>(pImageData);
		if (pTexture) return pTexture;

		Profiler::BeginSample("GPUResourceManager::CreateTexture");
		pTexture = CreateTexture_Internal(pImageData);
		pTexture->m_UUID = pImageData->GetGPUUUID();
		pTexture->Create(pImageData);
		m_IDResources[pImageData->GetGPUUUID()] = pTexture;
		Profiler::EndSample();
		return pTexture;
	}

	Texture* GPUResourceManager::CreateTexture(uint32_t width, uint32_t height, const PixelFormat& format, const PixelFormat& internalFormat, const ImageType& imageType, uint32_t usageFlags, uint32_t sharingMode, ImageAspect imageAspectFlags, const SamplerSettings& samplerSettings)
	{
		Profiler::BeginSample("GPUResourceManager::CreateTexture (2)");
		Texture* pTexture = CreateTexture_Internal(width, height, format, internalFormat, imageType, usageFlags, sharingMode, imageAspectFlags, samplerSettings);
		UUID id = UUID();
		pTexture->m_UUID = id;
		pTexture->Create();

		m_IDResources[id] = pTexture;
		Profiler::EndSample();
		return pTexture;
	}

	RenderTexture* GPUResourceManager::CreateRenderTexture(const RenderTextureCreateInfo& createInfo)
	{
		Profiler::BeginSample("GPUResourceManager::CreateRenderTexture");
		RenderTexture* pRenderTexture = CreateRenderTexture_Internal(createInfo);
		UUID id = UUID();
		pRenderTexture->m_UUID = id;
		pRenderTexture->Initialize();
		m_IDResources[id] = pRenderTexture;
		Profiler::EndSample();
		return pRenderTexture;
	}

	void GPUResourceManager::Free(GPUResource* pResource)
	{
		UUID id = pResource->m_UUID;
		auto it = m_IDResources.find(id);
		if (it == m_IDResources.end()) return;
		Profiler::BeginSample("GPUResourceManager::Free");
		delete m_IDResources[id];
		m_IDResources.erase(it);
		Profiler::EndSample();
	}

	bool GPUResourceManager::ResourceExists(Resource* pResource)
	{
		return m_IDResources.find(pResource->GetGPUUUID()) != m_IDResources.end();
	}
}