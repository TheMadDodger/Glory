#include "GPUResourceManager.h"
#include <algorithm>

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
		pMesh->SetBuffers(pVertexBuffer, pIndexBuffer);
		pMesh->CreateBindingAndAttributeData();
		m_IDResources[pMeshData->GetUUID()] = pMesh;
		return pMesh;
	}

	Shader* GPUResourceManager::CreateShader(FileData* pShaderFile, const ShaderType& shaderType, const std::string& function)
	{
		Shader* pShader = GetResource<Shader>(pShaderFile);
		if (pShader) return pShader;

		pShader = CreateShader_Internal(pShaderFile, shaderType, function);
		pShader->Initialize();
		m_IDResources[pShaderFile->GetUUID()] = pShader;
		return pShader;
	}

	Material* GPUResourceManager::CreateMaterial(MaterialData* pMaterialData)
	{
		Material* pMaterial = GetResource<Material>(pMaterialData);
		if (pMaterial) return pMaterial;

		pMaterial = CreateMaterial_Internal(pMaterialData);

		for (size_t i = 0; i < pMaterialData->ShaderCount(); i++)
		{
			FileData* pShaderFile = pMaterialData->GetShaderAt(i);
			const ShaderType& shaderType = pMaterialData->GetShaderTypeAt(i);
			Shader* pShader = CreateShader(pShaderFile, shaderType, "main");
			pMaterial->AddShader(pShader);
		}

		pMaterial->Initialize();
		m_IDResources[pMaterialData->GetUUID()] = pMaterial;
		return pMaterial;
	}

	Texture* GPUResourceManager::CreateTexture(ImageData* pImageData)
	{
		Texture* pTexture = GetResource<Texture>(pImageData);
		if (pTexture) return pTexture;

		pTexture = CreateTexture_Internal(pImageData);
		pTexture->Create(pImageData);
		m_IDResources[pImageData->GetUUID()] = pTexture;
		return pTexture;
	}
}