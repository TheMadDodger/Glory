#include "VulkanResourceManager.h"

#include "VulkanMesh.h"
#include "VulkanBuffer.h"
#include "VulkanShader.h"
#include "VulkanTexture.h"

#include <Engine.h>
#include <CubemapData.h>

namespace Glory
{
	VulkanResourceManager::VulkanResourceManager(Engine* pEngine): GPUResourceManager(pEngine)
	{
	}

	VulkanResourceManager::~VulkanResourceManager()
	{
	}

	Mesh* VulkanResourceManager::CreateMesh_Internal(MeshData* pMeshData)
	{
		const AttributeType* attributeTypes = pMeshData->AttributeTypes();
		std::vector<AttributeType> attributes = std::vector<AttributeType>(pMeshData->AttributeCount());
		for (size_t i = 0; i < attributes.size(); i++)
			attributes[i] = attributeTypes[i];

		return new VulkanMesh(pMeshData->VertexCount(), pMeshData->IndexCount(), InputRate::Vertex, 0, pMeshData->VertexSize(), attributes);
	}

	Mesh* VulkanResourceManager::CreateMesh_Internal(uint32_t vertexCount, uint32_t indexCount, InputRate inputRate, uint32_t binding, uint32_t stride, PrimitiveType primitiveType, const std::vector<AttributeType>& attributeTypes)
	{
		return new VulkanMesh(vertexCount, indexCount, inputRate, binding, stride, primitiveType, attributeTypes);
	}

	Buffer* VulkanResourceManager::CreateVertexBuffer(uint32_t bufferSize)
	{
		return CreateBuffer(bufferSize, BufferBindingTarget::B_ARRAY, MemoryUsage::MU_STATIC_DRAW, 0);
	}

	Buffer* VulkanResourceManager::CreateIndexBuffer(uint32_t bufferSize)
	{
		return CreateBuffer(bufferSize, BufferBindingTarget::B_ELEMENT_ARRAY, MemoryUsage::MU_STATIC_DRAW, 0);
	}

	Buffer* VulkanResourceManager::CreateBuffer_Internal(uint32_t bufferSize, BufferBindingTarget usageFlag, MemoryUsage memoryFlags, uint32_t bindIndex)
	{
		return new VulkanBuffer(bufferSize, usageFlag, memoryFlags, bindIndex);
	}

	Shader* VulkanResourceManager::CreateShader_Internal(const FileData* pShaderFile, const ShaderType& shaderType, const std::string& function)
	{
		return nullptr;
	}

	Material* VulkanResourceManager::CreateMaterial_Internal(MaterialData* pMaterialData)
	{
		return nullptr;
	}

	Pipeline* VulkanResourceManager::CreatePipeline_Internal(PipelineData* pPipelineData)
	{
		return nullptr;
	}

	Texture* VulkanResourceManager::CreateTexture_Internal(TextureData* pTextureData)
	{
		ImageData* pImageData = pTextureData->GetImageData(&m_pEngine->GetAssetManager());
		if (!pImageData) return nullptr;
		return new VulkanTexture({ pImageData->GetWidth(), pImageData->GetHeight(), pImageData->GetFormat(),
			pImageData->GetInternalFormat(), ImageType::IT_2D, pImageData->GetDataType(),
			0, 0, ImageAspect::IA_Color, pTextureData->GetSamplerSettings() });
	}

	Texture* VulkanResourceManager::CreateTexture_Internal(CubemapData* pCubemapData)
	{
		ImageData* pImageData = pCubemapData->GetImageData(&m_pEngine->GetAssetManager(), 0);
		if (!pImageData) return nullptr;
		return new VulkanTexture({ pImageData->GetWidth(), pImageData->GetHeight(), pImageData->GetFormat(),
			pImageData->GetInternalFormat(), ImageType::IT_Cube, pImageData->GetDataType(),
			0, 0, ImageAspect::IA_Color, pCubemapData->GetSamplerSettings() });
	}

	Texture* VulkanResourceManager::CreateTexture_Internal(TextureCreateInfo&& textureInfo)
	{
		return new VulkanTexture(std::move(textureInfo));
	}

	RenderTexture* VulkanResourceManager::CreateRenderTexture_Internal(const RenderTextureCreateInfo& createInfo)
	{
		return nullptr;
	}
}
