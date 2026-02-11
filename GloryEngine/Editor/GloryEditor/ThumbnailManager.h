#pragma once
#include "GloryEditor.h"

#include <map>

#include <GraphicsHandles.h>

namespace Glory
{
	class TextureData;
}

namespace Glory::Editor
{
	class BaseThumbnailGenerator;
	class EditorApplication;

	class ThumbnailManager
	{
	public:
		ThumbnailManager(EditorApplication* pApp);
		virtual ~ThumbnailManager();

		GLORY_EDITOR_API TextureHandle GetThumbnail(UUID uuid);
		GLORY_EDITOR_API void SetDirty(UUID uuid);

		template<class T>
		void AddGenerator()
		{
			BaseThumbnailGenerator* pGenerator = new T();
			AddGenerator(pGenerator);
		}

		GLORY_EDITOR_API BaseThumbnailGenerator* GetGenerator(uint32_t hashCode);

	private:
		GLORY_EDITOR_API void AddGenerator(BaseThumbnailGenerator* pGenerator);

	private:
		EditorApplication* m_pApplication;

		std::vector<BaseThumbnailGenerator*> m_pGenerators;
		std::map<UUID, TextureData*> m_pThumbnails;
	};
}
