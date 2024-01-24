#include "OGLResourceManager.h"
#include "GLBuffer.h"
#include "GLMesh.h"
#include "GLShader.h"
#include "OGLMaterial.h"
#include "GLTexture.h"
#include "OGLRenderTexture.h"
#include "GloryOGL.h"

#include <Engine.h>
#include <GL/glew.h>

namespace Glory
{
	OGLResourceManager::OGLResourceManager(Engine* pEngine): GPUResourceManager(pEngine)
	{
	}

	OGLResourceManager::~OGLResourceManager()
	{
	}

	Mesh* OGLResourceManager::CreateMesh_Internal(MeshData* pMeshData)
	{
		const AttributeType* attributeTypes = pMeshData->AttributeTypes();
		std::vector<AttributeType> attributes = std::vector<AttributeType>(pMeshData->AttributeCount());
		for (size_t i = 0; i < attributes.size(); i++)
			attributes[i] = attributeTypes[i];

		return new GLMesh(pMeshData->VertexCount(), pMeshData->IndexCount(), InputRate::Vertex, 0, pMeshData->VertexSize(), attributes);
	}

	Mesh* OGLResourceManager::CreateMesh_Internal(uint32_t vertexCount, uint32_t indexCount, InputRate inputRate, uint32_t binding, uint32_t stride, PrimitiveType primitiveType, const std::vector<AttributeType>& attributeTypes)
	{
		return new GLMesh(vertexCount, indexCount, inputRate, binding, stride, primitiveType, attributeTypes);
	}

	Buffer* OGLResourceManager::CreateVertexBuffer(uint32_t bufferSize)
	{
		return CreateBuffer(bufferSize, BufferBindingTarget::B_ARRAY, MemoryUsage::MU_STATIC_DRAW, 0);
	}

	Buffer* OGLResourceManager::CreateIndexBuffer(uint32_t bufferSize)
	{
		return CreateBuffer(bufferSize, BufferBindingTarget::B_ELEMENT_ARRAY, MemoryUsage::MU_STATIC_DRAW, 0);
	}

	Buffer* OGLResourceManager::CreateBuffer_Internal(uint32_t bufferSize, BufferBindingTarget usageFlag, MemoryUsage memoryFlags, uint32_t bindIndex)
	{
		return new GLBuffer(bufferSize, usageFlag, memoryFlags, bindIndex);
	}

	Shader* OGLResourceManager::CreateShader_Internal(FileData* pShaderFile, const ShaderType& shaderType, const std::string& function)
	{
		return new GLShader(pShaderFile, shaderType, function);
	}

	Material* OGLResourceManager::CreateMaterial_Internal(MaterialData* pMaterialData)
	{
		return new OGLMaterial(pMaterialData);
	}

	Texture* OGLResourceManager::CreateTexture_Internal(TextureData* pTextureData)
	{
		Resource* pParent = pTextureData->ParentResource();
		ImageData* pParentImage = pParent ? dynamic_cast<ImageData*>(pParent) : nullptr;
		ImageData* pImageData = pParentImage ? pParentImage : pTextureData->Image().Get(&m_pEngine->GetResources());
		if (!pImageData) return nullptr;
		return new GLTexture(pImageData->GetWidth(), pImageData->GetHeight(), pImageData->GetFormat(), pImageData->GetInternalFormat(), ImageType::IT_2D, 0, 0, ImageAspect::IA_Color);
	}

	Texture* OGLResourceManager::CreateTexture_Internal(uint32_t width, uint32_t height, const PixelFormat& format, const PixelFormat& internalFormat, const ImageType& imageType, uint32_t usageFlags, uint32_t sharingMode, ImageAspect imageAspectFlags, const SamplerSettings& samplerSettings)
	{
		return new GLTexture(width, height, format, internalFormat, imageType, usageFlags, sharingMode, imageAspectFlags, samplerSettings);
	}

	RenderTexture* OGLResourceManager::CreateRenderTexture_Internal(const RenderTextureCreateInfo& createInfo)
	{
		return new OGLRenderTexture(createInfo);
	}
}