#pragma once
#include "visibility.h"

#include <functional>
#include <Version.h>
#include <vector>

#define NOMINMAX

namespace Glory
{
	class GloryAPI
	{
	public:
		GLORY_API_API static bool Initialize();
		GLORY_API_API static void Cleanup();
		GLORY_API_API static void RunRequests();

	public: /* API Calls */
		/*
		 * @brief Fetch latest editor version
		 * @param callback The callback that will be called once the API request succeeds
		 */
		GLORY_API_API static void FetchEditorVersion(std::function<void(const Version&)> callback);

	private:
		static Version FetchEditorVersion_Impl();

	private:

		static void Run(std::function<bool()> func);
		static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
	};
}
