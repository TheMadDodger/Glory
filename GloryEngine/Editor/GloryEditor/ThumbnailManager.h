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

	class ThumbnailManager
	{
	public:
		static GLORY_EDITOR_API TextureHandle GetThumbnail(UUID uuid);
		static GLORY_EDITOR_API void SetDirty(UUID uuid);

		template<class T>
		static void AddGenerator()
		{
			BaseThumbnailGenerator* pGenerator = new T();
			AddGenerator(pGenerator);
		}

		static GLORY_EDITOR_API void Destroy();
		static GLORY_EDITOR_API BaseThumbnailGenerator* GetGenerator(uint32_t hashCode);

	private:
		static GLORY_EDITOR_API void AddGenerator(BaseThumbnailGenerator* pGenerator);

	private:
		static std::vector<BaseThumbnailGenerator*> m_pGenerators;
		static std::map<UUID, TextureData*> m_pThumbnails;

	private:
		ThumbnailManager();
		virtual ~ThumbnailManager();
	};
}
