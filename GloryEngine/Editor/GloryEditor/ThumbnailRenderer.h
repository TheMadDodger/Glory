#pragma once
#include <SceneManager.h>
#include <GloryEditor.h>
#include <Entity.h>
#include <GraphicsHandles.h>

#include <BitSet.h>

#include <functional>
#include <queue>
#include <set>
#include <unordered_map>

namespace Glory
{
	class Engine;
	class MeshData;
	class ImageData;
	class InternalTexture;
	class TextureData;
	class MaterialData;

	namespace Editor
	{
		inline MeshData* MaterialSphereMesh = nullptr;
		inline MaterialData* GreyMaterial = nullptr;

		class ThumbnailRenderer : public SceneManager
		{
		public:
			ThumbnailRenderer(Engine* pEngine);
			virtual ~ThumbnailRenderer();

			GLORY_EDITOR_API void RegisterRenderableThumbnail(uint32_t hashCode,
				std::function<void(Entity, UUID)> sceneSetup, std::function<bool(UUID)> canRender);
			GLORY_EDITOR_API bool IsResourceRenderable(uint32_t hashCode);
			GLORY_EDITOR_API bool CanRenderThumbnail(uint32_t hashCode, UUID id);
			GLORY_EDITOR_API TextureData* QueueThumbnailForRendering(uint32_t hashCode, UUID id);

			void LoadResources();
			void CheckRenders();

			void RerenderThumbnail(UUID uuid);

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
			std::map<uint32_t, std::function<bool(UUID)>> m_CanRenderThumbnailCallbacks;

			struct QueuedThumbnail
			{
				uint32_t AssetHash;
				UUID AssetID;
			};

			std::set<UUID> m_PreviouslyRequestedThumbnails;
			std::queue<std::pair<uint32_t, UUID>> m_QueuedThumbnails;

			static const size_t MaxThumbnailsInFlight = 5;
			UUID m_RenderingIDs[MaxThumbnailsInFlight];

			std::vector<ImageData*> m_RenderResults;
			std::unordered_map<UUID, InternalTexture*> m_RenderResultTextures;

			BufferHandle m_PixelCopyBuffer;
		};
	}
}