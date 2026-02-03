#pragma once
#include "GloryEditor.h"

#include <map>

#include <GraphicsHandles.h>
#include <Entity.h>

namespace Glory
{
	class TextureData;
}

namespace Glory::Editor
{
	class BaseThumbnailGenerator;
	class EditorApplication;
	class ThumbnailRenderer;

	class ThumbnailManager
	{
	public:
		ThumbnailManager(EditorApplication* pApp);
		virtual ~ThumbnailManager();

		GLORY_EDITOR_API TextureHandle GetThumbnail(UUID uuid);
		GLORY_EDITOR_API void SetDirty(UUID uuid);
		GLORY_EDITOR_API void SetupInternalRenderableThumbnails();

		template<class T>
		void AddGenerator()
		{
			BaseThumbnailGenerator* pGenerator = new T();
			AddGenerator(pGenerator);
		}

		template<class T>
		void RegisterRenderableThumbnail(std::function<void(Entity, UUID)> sceneSetup)
		{
			const uint32_t hash = ResourceTypes::GetHash<T>();
			RegisterRenderableThumbnail(hash, sceneSetup);
		}

		GLORY_EDITOR_API BaseThumbnailGenerator* GetGenerator(uint32_t hashCode);
		GLORY_EDITOR_API void Initialize();
		GLORY_EDITOR_API void Update();

	private:
		GLORY_EDITOR_API void AddGenerator(BaseThumbnailGenerator* pGenerator);
		GLORY_EDITOR_API void RegisterRenderableThumbnail(uint32_t hashCode, std::function<void(Entity, UUID)> sceneSetup);

	private:
		EditorApplication* m_pApplication;

		std::unique_ptr<ThumbnailRenderer> m_ThumbnailRenderer;
		std::vector<BaseThumbnailGenerator*> m_pGenerators;
		std::map<UUID, TextureData*> m_pThumbnails;
	};
}
