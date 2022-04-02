#include "VulkanResourceManager.h"

namespace Glory
{
	VulkanResourceManager::VulkanResourceManager()
	{
	}

	VulkanResourceManager::~VulkanResourceManager()
	{
	}

	Mesh* VulkanResourceManager::CreateMesh_Internal(MeshData* pMeshData)
	{
		return nullptr;
	}

	Buffer* VulkanResourceManager::CreateVertexBuffer(uint32_t bufferSize)
	{
		return nullptr;
	}

	Buffer* VulkanResourceManager::CreateIndexBuffer(uint32_t bufferSize)
	{
		return nullptr;
	}

	Buffer* VulkanResourceManager::CreateBuffer_Internal(uint32_t bufferSize, uint32_t usageFlag, uint32_t memoryFlags, uint32_t bindIndex)
	{
		return nullptr;
	}

	Shader* VulkanResourceManager::CreateShader_Internal(FileData* pShaderFile, const ShaderType& shaderType, const std::string& function)
	{
		return nullptr;
	}

	Material* VulkanResourceManager::CreateMaterial_Internal(MaterialData* pMaterialData)
	{
		return nullptr;
	}

	Texture* VulkanResourceManager::CreateTexture_Internal(ImageData* pImageData)
	{
		return nullptr;
	}

	Texture* VulkanResourceManager::CreateTexture_Internal(uint32_t width, uint32_t height, const PixelFormat& format, const PixelFormat& internalFormat, const ImageType& imageType, uint32_t usageFlags, uint32_t sharingMode, ImageAspect imageAspectFlags, const SamplerSettings& samplerSettings)
	{
		return nullptr;
	}

	RenderTexture* VulkanResourceManager::CreateRenderTexture_Internal(const RenderTextureCreateInfo& createInfo)
	{
		return nullptr;
	}
}
