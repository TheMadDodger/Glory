#pragma once
#include "GloryEditor.h"

#include <Entity.h>
#include <ResourceType.h>
#include <GraphicsHandles.h>
#include <InternalTexture.h>

#include <memory>
#include <functional>
#include <filesystem>

namespace Glory
{
	class ImageData;
	class GraphicsDevice;
	class Renderer;
}

namespace Glory::Editor
{
	class EditorApplication;
	class ThumbnailsRenderer;

	struct ThumbnailData
	{
		const TextureHandle Texture = nullptr;
		const std::string_view Icon = nullptr;
	};

	class Thumbnails
	{
	public:
		Thumbnails(EditorApplication* pApp);
		virtual ~Thumbnails();

		template<class T>
		void RegisterRenderableThumbnail(std::function<void(Entity, UUID)> sceneSetup, std::function<bool(UUID)> canRender=NULL,
			std::function<void(UUID, GraphicsDevice*, Renderer*, uint32_t, CommandBufferHandle)> customRender=NULL)
		{
			const uint32_t hash = ResourceTypes::GetHash<T>();
			RegisterRenderableThumbnail(hash, sceneSetup, canRender, customRender);
		}

		GLORY_EDITOR_API void Initialize();
		GLORY_EDITOR_API void Update();

		GLORY_EDITOR_API void SetupInternalRenderableThumbnails();
		GLORY_EDITOR_API bool DrawThumbnailButton(UUID uuid, float size);
		GLORY_EDITOR_API void DrawThumbnail(UUID uuid, float size);
		GLORY_EDITOR_API void SetDirty(UUID uuid);

	private:
		GLORY_EDITOR_API void RegisterRenderableThumbnail(uint32_t hashCode, std::function<void(Entity, UUID)> sceneSetup,
			std::function<bool(UUID)> canRender=NULL, std::function<void(UUID, GraphicsDevice*, Renderer*, uint32_t, CommandBufferHandle)> customRender=NULL);
		GLORY_EDITOR_API std::filesystem::path GenerateCachedThumbnailPath(const UUID uuid) const;
		GLORY_EDITOR_API ThumbnailData GetThumbnail(const UUID uuid);

	private:
		EditorApplication* m_pApp;
		std::unique_ptr<ThumbnailsRenderer> m_ThumbnailsRenderer;
		std::unordered_map<UUID, InternalTexture> m_CachedThumbnailTextures;
		std::unordered_map<UUID, UUID> m_ResourceIDToThumbnailID;
		std::unordered_map<UUID, TextureHandle> m_CachedThumbnailTextureHandles;
		std::unordered_map<UUID, uint8_t> m_CurrentRenderingThumbnails;
	};
}
