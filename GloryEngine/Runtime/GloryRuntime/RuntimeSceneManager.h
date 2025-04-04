#pragma once
#include "Visibility.h"

#include <SceneManager.h>

namespace std::filesystem
{
	class path;
}

namespace Glory
{
	class GScene;
	class GloryRuntime;

	class RuntimeSceneManager : public SceneManager
	{
	public:
		RuntimeSceneManager(GloryRuntime* pRuntime);
		virtual ~RuntimeSceneManager();

		GLORY_RUNTIME_API GScene* NewScene(const std::string& name = "Empty Scene", bool additive = false);
		GLORY_RUNTIME_API void OnLoadScene(UUID uuid) override;
		GLORY_RUNTIME_API void OnUnloadScene(GScene* pScene) override;
		GLORY_RUNTIME_API void OnUnloadAllScenes() override;

	private:
		/** @brief Load a scene and its assets and shaders */
		GLORY_RUNTIME_API void LoadScene(const std::filesystem::path& path);
		/** @brief Load a scene at a path */
		GLORY_RUNTIME_API void LoadSceneOnly(const std::filesystem::path& path);

		virtual void OnInitialize() override {}
		virtual void OnCleanup() override {}
		virtual void OnSetActiveScene(GScene*) override {}

	private:
		GloryRuntime* m_pRuntime;
	};
}
