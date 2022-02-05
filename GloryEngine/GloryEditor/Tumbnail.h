#pragma once
#include <map>
#include "ImageData.h"
#include "Texture.h"

namespace Glory::Editor
{
	class BaseTumbnailGenerator;

	class Tumbnail
	{
	public:
		static Texture* GetTumbnail(UUID uuid);

		template<class T>
		static void AddGenerator()
		{
			BaseTumbnailGenerator* pGenerator = new T();
			AddGenerator(pGenerator);
		}

		static void Destroy();
		static BaseTumbnailGenerator* GetGenerator(size_t hashCode);

	private:
		static void AddGenerator(BaseTumbnailGenerator* pGenerator);

	private:
		static std::vector<BaseTumbnailGenerator*> m_pGenerators;
		static std::map<UUID, ImageData*> m_pTumbnails;

	private:
		Tumbnail();
		virtual ~Tumbnail();
	};
}
