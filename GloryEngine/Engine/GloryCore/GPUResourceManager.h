#pragma once
#include <map>
#include <vector>
#include "UUID.h"
#include "GPUResource.h"
#include "Buffer.h"
#include "Mesh.h"
#include "Shader.h"
#include "Material.h"
#include "Texture.h"
#include "RenderTexture.h"
#include "TextureData.h"

namespace Glory
{
	class GPUResourceManager
	{
	public:
		GPUResourceManager();
		virtual ~GPUResourceManager();

	public:
		Buffer* CreateBuffer(uint32_t bufferSize, BufferBindingTarget usageFlag, MemoryUsage memoryFlags, uint32_t bindIndex);
		Mesh* CreateMesh(MeshData* pMeshData);
		Mesh* CreateMeshNoIndexBuffer(MeshData* pMeshData);
		Mesh* CreateMesh(uint32_t vertexCount, uint32_t indexCount, InputRate inputRate, uint32_t binding, uint32_t stride, PrimitiveType primitiveType, const std::vector<AttributeType>& attributeTypes, Buffer* pVertexBuffer, Buffer* pIndexBuffer);
		Shader* CreateShader(FileData* pShaderFile, const ShaderType& shaderType, const std::string& function);
		Material* CreateMaterial(MaterialData* pMaterialData);
		Texture* CreateTexture(TextureData* pTextureData);
		Texture* CreateTexture(uint32_t width, uint32_t height, const PixelFormat& format, const PixelFormat& internalFormat, const ImageType& imageType, uint32_t usageFlags, uint32_t sharingMode, ImageAspect imageAspectFlags, const SamplerSettings& samplerSettings);
		RenderTexture* CreateRenderTexture(const RenderTextureCreateInfo& createInfo);

		void Free(GPUResource* pResource);
		void Free(Buffer* pBuffer);

		bool ResourceExists(Resource* pResource);

	protected: // Internal functions
		virtual Buffer* CreateVertexBuffer(uint32_t bufferSize) = 0;
		virtual Buffer* CreateIndexBuffer(uint32_t bufferSize) = 0;
		virtual Buffer* CreateBuffer_Internal(uint32_t bufferSize, BufferBindingTarget usageFlag, MemoryUsage memoryFlags, uint32_t bindIndex) = 0;
		virtual Mesh* CreateMesh_Internal(MeshData* pMeshData) = 0;
		virtual Mesh* CreateMesh_Internal(uint32_t vertexCount, uint32_t indexCount, InputRate inputRate, uint32_t binding, uint32_t stride, PrimitiveType primitiveType, const std::vector<AttributeType>& attributeTypes) = 0;
		virtual Shader* CreateShader_Internal(FileData* pShaderFile, const ShaderType& shaderType, const std::string& function) = 0;
		virtual Material* CreateMaterial_Internal(MaterialData* pMaterialData) = 0;
		virtual Texture* CreateTexture_Internal(TextureData* pTextureData) = 0;
		virtual Texture* CreateTexture_Internal(uint32_t width, uint32_t height, const PixelFormat& format, const PixelFormat& internalFormat, const ImageType& imageType, uint32_t usageFlags, uint32_t sharingMode, ImageAspect imageAspectFlags, const SamplerSettings& samplerSettings) = 0;
		virtual RenderTexture* CreateRenderTexture_Internal(const RenderTextureCreateInfo& createInfo) = 0;

	private: // Resource lookups
		template<class T>
		T* GetResource(Resource* pResource)
		{
			const UUID& uuid = pResource->GetGPUUUID();
			auto it = m_IDResources.find(uuid);
			if (it == m_IDResources.end()) return nullptr;
			return (T*)m_IDResources[uuid];
		}

	private:
		std::map<UUID, Buffer*> m_pBuffers;
		std::map<UUID, GPUResource*> m_IDResources;
	};
}
