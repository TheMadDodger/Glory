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

namespace Glory
{
	class GPUResourceManager
	{
	public:
		GPUResourceManager();
		virtual ~GPUResourceManager();

	public:
		Buffer* CreateBuffer(uint32_t bufferSize, uint32_t usageFlag, uint32_t memoryFlags);
		Mesh* CreateMesh(MeshData* pMeshData);
		Shader* CreateShader(FileData* pShaderFile, const ShaderType& shaderType, const std::string& function);
		Material* CreateMaterial(MaterialData* pMaterialData);
		Texture* CreateTexture(ImageData* pImageData);
		Texture* CreateTexture(uint32_t width, uint32_t height, const PixelFormat& format, const ImageType& imageType, uint32_t usageFlags, uint32_t sharingMode, ImageAspect imageAspectFlags, const SamplerSettings& samplerSettings);
		RenderTexture* CreateRenderTexture(int width, int height, bool hasDepthBuffer);

		void Free(GPUResource* pResource);

		bool ResourceExists(Resource* pResource);

	protected: // Internal functions
		virtual Buffer* CreateVertexBuffer(uint32_t bufferSize) = 0;
		virtual Buffer* CreateIndexBuffer(uint32_t bufferSize) = 0;
		virtual Buffer* CreateBuffer_Internal(uint32_t bufferSize, uint32_t usageFlag, uint32_t memoryFlags) = 0;
		virtual Mesh* CreateMesh_Internal(MeshData* pMeshData) = 0;
		virtual Shader* CreateShader_Internal(FileData* pShaderFile, const ShaderType& shaderType, const std::string& function) = 0;
		virtual Material* CreateMaterial_Internal(MaterialData* pMaterialData) = 0;
		virtual Texture* CreateTexture_Internal(ImageData* pImageData) = 0;
		virtual Texture* CreateTexture_Internal(uint32_t width, uint32_t height, const PixelFormat& format, const ImageType& imageType, uint32_t usageFlags, uint32_t sharingMode, ImageAspect imageAspectFlags, const SamplerSettings& samplerSettings) = 0;
		virtual RenderTexture* CreateRenderTexture_Internal(int width, int height, bool hasDepthBuffer) = 0;

	private: // Resource lookups
		template<class T>
		T* GetResource(Resource* pResource)
		{
			const UUID& uuid = pResource->GetUUID();
			auto it = m_IDResources.find(uuid);
			if (it == m_IDResources.end()) return nullptr;
			return (T*)m_IDResources[uuid];
		}

	private:
		std::vector<Buffer*> m_pBuffers;
		std::map<UUID, GPUResource*> m_IDResources;
	};
}
