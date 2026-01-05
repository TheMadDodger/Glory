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
	class BaseTumbnailGenerator;

	class Tumbnail
	{
	public:
		static GLORY_EDITOR_API TextureHandle GetTumbnail(UUID uuid);
		static GLORY_EDITOR_API void SetDirty(UUID uuid);

		template<class T>
		static void AddGenerator()
		{
			BaseTumbnailGenerator* pGenerator = new T();
			AddGenerator(pGenerator);
		}

		static GLORY_EDITOR_API void Destroy();
		static GLORY_EDITOR_API BaseTumbnailGenerator* GetGenerator(uint32_t hashCode);

	private:
		static GLORY_EDITOR_API void AddGenerator(BaseTumbnailGenerator* pGenerator);

	private:
		static std::vector<BaseTumbnailGenerator*> m_pGenerators;
		static std::map<UUID, TextureData*> m_pTumbnails;

	private:
		Tumbnail();
		virtual ~Tumbnail();
	};
}
