#include "OGLResourceManager.h"
#include "GLBuffer.h"
#include "GLMesh.h"
#include "GLShader.h"
#include "OGLMaterial.h"
#include "OGLPipeline.h"
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

	Pipeline* OGLResourceManager::CreatePipeline_Internal(PipelineData* pPipelineData)
	{
		return new OGLPipeline(pPipelineData);
	}

	Texture* OGLResourceManager::CreateTexture_Internal(TextureData* pTextureData)
	{
		ImageData* pImageData = pTextureData->GetImageData(&m_pEngine->GetAssetManager());
		if (!pImageData) return nullptr;
		return new GLTexture({ pImageData->GetWidth(), pImageData->GetHeight(), pImageData->GetFormat(), pImageData->GetInternalFormat(), ImageType::IT_2D, DataType::DT_UByte, 0, 0, ImageAspect::IA_Color });
	}

	Texture* OGLResourceManager::CreateTexture_Internal(TextureCreateInfo&& textureInfo)
	{
		return new GLTexture(std::move(textureInfo));
	}

	RenderTexture* OGLResourceManager::CreateRenderTexture_Internal(const RenderTextureCreateInfo& createInfo)
	{
		return new OGLRenderTexture(createInfo);
	}
}