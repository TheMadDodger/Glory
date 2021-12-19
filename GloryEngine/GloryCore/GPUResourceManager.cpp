#include "GPUResourceManager.h"
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

	Buffer* GPUResourceManager::CreateBuffer(uint32_t bufferSize, uint32_t usageFlag, uint32_t memoryFlags)
	{
		Buffer* pBuffer = CreateBuffer_Internal(bufferSize, usageFlag, memoryFlags);
		pBuffer->CreateBuffer();
		m_pBuffers.push_back(pBuffer);
		return pBuffer;
	}

	Mesh* GPUResourceManager::CreateMesh(MeshData* pMeshData)
	{
		Mesh* pMesh = GetResource<Mesh>(pMeshData);
		if (pMesh) return pMesh;


		uint32_t vertexBufferSize = pMeshData->VertexCount() * pMeshData->VertexSize();
		uint32_t indexBufferSize = pMeshData->IndexCount() * sizeof(uint32_t);
		Buffer* pVertexBuffer = CreateVertexBuffer(vertexBufferSize);
		Buffer* pIndexBuffer = CreateIndexBuffer(indexBufferSize);
		pVertexBuffer->Assign(pMeshData->Vertices());
		pIndexBuffer->Assign(pMeshData->Indices());
		pMesh = CreateMesh_Internal(pMeshData);
		pMesh->m_UUID = pMeshData->GetGPUUUID();
		pMesh->SetBuffers(pVertexBuffer, pIndexBuffer);
		pMesh->CreateBindingAndAttributeData();
		m_IDResources[pMeshData->GetGPUUUID()] = pMesh;
		return pMesh;
	}

	Shader* GPUResourceManager::CreateShader(FileData* pShaderFile, const ShaderType& shaderType, const std::string& function)
	{
		Shader* pShader = GetResource<Shader>(pShaderFile);
		if (pShader) return pShader;

		pShader = CreateShader_Internal(pShaderFile, shaderType, function);
		pShader->m_UUID = pShaderFile->GetGPUUUID();
		pShader->Initialize();
		m_IDResources[pShaderFile->GetGPUUUID()] = pShader;
		return pShader;
	}

	Material* GPUResourceManager::CreateMaterial(MaterialData* pMaterialData)
	{
		Material* pMaterial = GetResource<Material>(pMaterialData);
		if (pMaterial)
		{
			pMaterial->m_pMaterialData = pMaterialData;
			return pMaterial;
		}

		pMaterial = CreateMaterial_Internal(pMaterialData);
		pMaterial->m_pMaterialData = pMaterialData;
		pMaterial->m_UUID = pMaterialData->GetGPUUUID();
		for (size_t i = 0; i < pMaterialData->ShaderCount(); i++)
		{
			FileData* pShaderFile = pMaterialData->GetShaderAt(i);
			const ShaderType& shaderType = pMaterialData->GetShaderTypeAt(i);
			Shader* pShader = CreateShader(pShaderFile, shaderType, "main");
			pMaterial->AddShader(pShader);
		}

		pMaterial->Initialize();
		m_IDResources[pMaterialData->GetGPUUUID()] = pMaterial;
		return pMaterial;
	}

	Texture* GPUResourceManager::CreateTexture(ImageData* pImageData)
	{
		Texture* pTexture = GetResource<Texture>(pImageData);
		if (pTexture) return pTexture;

		pTexture = CreateTexture_Internal(pImageData);
		pTexture->m_UUID = pImageData->GetGPUUUID();
		pTexture->Create(pImageData);
		m_IDResources[pImageData->GetGPUUUID()] = pTexture;
		return pTexture;
	}

	Texture* GPUResourceManager::CreateTexture(uint32_t width, uint32_t height, const PixelFormat& format, const ImageType& imageType, uint32_t usageFlags, uint32_t sharingMode, ImageAspect imageAspectFlags, const SamplerSettings& samplerSettings)
	{
		Texture* pTexture = CreateTexture_Internal(width, height, format, imageType, usageFlags, sharingMode, imageAspectFlags, samplerSettings);
		UUID id = UUID();
		pTexture->m_UUID = id;
		pTexture->Create();

		m_IDResources[id] = pTexture;
		return pTexture;
	}

	RenderTexture* GPUResourceManager::CreateRenderTexture(int width, int height, bool hasDepthBuffer)
	{
		RenderTexture* pRenderTexture = CreateRenderTexture_Internal(width, height, hasDepthBuffer);
		UUID id = UUID();
		pRenderTexture->m_UUID = id;
		pRenderTexture->Initialize();
		m_IDResources[id] = pRenderTexture;
		return pRenderTexture;
	}

	void GPUResourceManager::Free(GPUResource* pResource)
	{
		UUID id = pResource->m_UUID;
		auto it = m_IDResources.find(id);
		if (it == m_IDResources.end()) return;
		delete m_IDResources[id];
		m_IDResources.erase(it);
	}

	bool GPUResourceManager::ResourceExists(Resource* pResource)
	{
		return m_IDResources.find(pResource->GetGPUUUID()) != m_IDResources.end();
	}
}