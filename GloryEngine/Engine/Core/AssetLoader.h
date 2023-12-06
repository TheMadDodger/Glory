#pragma once
#include "UUID.h"
#include "AssetArchive.h"

#include <filesystem>
#include <JobManager.h>
#include <ThreadedVar.h>

namespace Glory
{
	class Engine;

	class AssetLoader
	{
	public:
		/** @brief Constructor */
		AssetLoader(Engine* pEngine);
		~AssetLoader();

		/** @brief Request to load an asset by UUID
		 * @param uuid ID of the asset to load
		 */
		bool RequestLoad(UUID uuid);

		/** @brief Dump all resources from loaded archives to resource managers */
		void DumpLoadedArchives();

	private:
		/** @brief Helper for running load jobs */
		bool LoadJob(const std::filesystem::path path);

		/** @brief Load an AssetArchive (.gcag file) at a path */
		AssetArchive LoadArchive(const std::filesystem::path& path);

	private:
		Engine* m_pEngine;
		ThreadedVector<AssetArchive> m_LoadedArchives;
		std::vector<std::filesystem::path> m_LoadingPaths;
		Jobs::JobPool<bool, const std::filesystem::path>* m_pLoadJobs;
	};
}
