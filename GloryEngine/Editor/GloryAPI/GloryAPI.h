#pragma once
#include "visibility.h"

namespace Glory
{
	class GloryAPI
	{
	public:
		GLORY_API_API static bool Initialize();
		GLORY_API_API static void Cleanup();

		GLORY_API_API static void Test();

	private:
		GLORY_API_API static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
	};
}
