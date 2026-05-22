#pragma once
#include "GloryEditor.h"

#include <Entity.h>
#include <ResourceType.h>

#include <memory>
#include <functional>
#include <filesystem>

namespace Glory
{
	class ImageData;
}

namespace Glory::Editor
{
	class EditorApplication;
	class ThumbnailRenderer;

	//struct ThumbnailData
	//{
	//	const TextureHandle Texture = nullptr;
	//	const std::string_view Icon = nullptr;
	//};

	class Thumbnails
	{
	public:
		Thumbnails(EditorApplication* pApp);
		virtual ~Thumbnails();

		template<class T>
		void RegisterRenderableThumbnail(std::function<void(Entity, UUID)> sceneSetup, std::function<bool(UUID)> canRender)
		{
			const uint32_t hash = ResourceTypes::GetHash<T>();
			RegisterRenderableThumbnail(hash, sceneSetup, canRender);
		}

		GLORY_EDITOR_API void Initialize();
		GLORY_EDITOR_API void Update();

		GLORY_EDITOR_API void SetupInternalRenderableThumbnails();
		GLORY_EDITOR_API void DrawThumbnail(UUID uuid, float size);

	private:
		GLORY_EDITOR_API void RegisterRenderableThumbnail(uint32_t hashCode,
			std::function<void(Entity, UUID)> sceneSetup, std::function<bool(UUID)> canRender);
		GLORY_EDITOR_API std::filesystem::path GenerateCachedThumbnailPath(const UUID uuid) const;

	private:
		EditorApplication* m_pApp;
		std::unique_ptr<ThumbnailRenderer> m_ThumbnailRenderer;
		std::unordered_map<UUID, ImageData*> m_CachedThumbnail;
	};
}
