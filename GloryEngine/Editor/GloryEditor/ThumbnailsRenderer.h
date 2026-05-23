#pragma once
#include "GloryEditor.h"

#include <SceneManager.h>
#include <Entity.h>
#include <GraphicsHandles.h>
#include <AssetReference.h>

#include <functional>
#include <queue>
#include <set>
#include <unordered_map>

#include <glm/vec2.hpp>

namespace Glory
{
	class IEngine;
	class ImageData;
	class InternalTexture;
	class TextureData;
	class GraphicsDevice;
}

namespace Glory::Editor
{
	inline UUID MaterialSphereMeshID = NULL;
	inline UUID GreyMaterialID = NULL;
	inline DescriptorSetLayoutHandle ImageSetLayoutHandle = NULL;
	inline PipelineHandle ImagePipelineHandle = NULL;
	inline std::vector<DescriptorSetHandle> ImageSetHandles;

	static inline constexpr glm::uvec2 ThumbnailResolution{ 128, 128 };
	static inline const size_t ThumbnailDataSize = ThumbnailResolution.x*ThumbnailResolution.y*4;

	class ThumbnailsRenderer : public SceneManager
	{
	public:
		ThumbnailsRenderer(IEngine* pEngine);
		virtual ~ThumbnailsRenderer();

		GLORY_EDITOR_API void RegisterRenderableThumbnail(uint32_t hashCode, std::function<void(Entity, UUID)> sceneSetup,
			std::function<bool(UUID)> canRender, std::function<void(UUID, GraphicsDevice*, Renderer*, uint32_t, CommandBufferHandle)> customRender);
		GLORY_EDITOR_API bool IsResourceRenderable(uint32_t hashCode);
		GLORY_EDITOR_API void QueueRenderThumbnail(uint32_t hashCode, UUID uuid);
		GLORY_EDITOR_API ImageData* GetRenderResult(UUID uuid);

		void LoadResources();
		void CheckRenders();

	private:
		GScene* NewScene(const std::string& name = "Empty Scene", bool additive = false);
		void OnLoadScene(UUID uuid) override;
		void OnUnloadScene(GScene* pScene) override;
		void OnUnloadAllScenes() override;

	private:
		virtual void OnInitialize() override {}
		virtual void OnCleanup() override {}
		virtual void OnSetActiveScene(GScene*) override {}

		void SetupRenders();

	private:
		std::map<uint32_t, std::function<void(Entity, UUID)>> m_ThumbnailRenderSetupCallbacks;
		std::map<uint32_t, std::function<bool(UUID)>> m_CanRenderThumbnailCallback;
		std::map<uint32_t, std::function<void(UUID, GraphicsDevice*, Renderer*, uint32_t, CommandBufferHandle)>> m_CustomRenderThumbnailCallbacks;

		BufferHandle m_PixelCopyBuffer;

		std::unordered_map<UUID, ResourceReferenceBase> m_References;
		std::vector<std::pair<uint32_t, UUID>> m_ToQueueThumbnails;
		std::queue<std::pair<uint32_t, UUID>> m_QueuedThumbnails;
		std::unordered_map<UUID, ImageData> m_RenderResults;

		static const size_t MaxThumbnailsInFlight = 1;
		UUID m_RenderingIDs[MaxThumbnailsInFlight];
	};
}
