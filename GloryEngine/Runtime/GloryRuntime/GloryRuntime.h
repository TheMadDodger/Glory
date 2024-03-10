#pragma once
#include "Visibility.h"

#include <UUID.h>
#include <memory>
#include <vector>
#include <string>

namespace std::filesystem
{
	class path;
}

namespace Glory
{
	class Engine;
	class RuntimeSceneManager;
	class RuntimeMaterialManager;
	class RuntimeShaderManager;
	class RendererModule;
	class GraphicsModule;
	class WindowModule;

	/** @brief Glory runtime */
	class GloryRuntime
	{
	public:
		/** @brief Constructor */
		GLORY_RUNTIME_API GloryRuntime(Engine* pEngine);
		/** @brief Destructor */
		GLORY_RUNTIME_API ~GloryRuntime();
		/** @brief Initialize the runtime */
		GLORY_RUNTIME_API void Initialize();
		/** @brief Run the runtime */
		GLORY_RUNTIME_API void Run();
		/** @brief Load an asset database at a path and append it to the current database */
		GLORY_RUNTIME_API void LoadAssetDatabase(const std::filesystem::path& path);
		/** @brief Load a scene and its assets and shaders */
		GLORY_RUNTIME_API void LoadScene(const UUID uuid);
		/** @brief Load an asset group at a path */
		GLORY_RUNTIME_API void LoadAssetGroup(const std::filesystem::path& path);
		/** @brief Load a shader pack at a path */
		GLORY_RUNTIME_API void LoadShaderPack(const std::filesystem::path& path);
		/** @brief Get the engine attached to this runtime */
		GLORY_RUNTIME_API Engine* GetEngine();
		/** @brief Set the data path */
		GLORY_RUNTIME_API void SetDataPath(const std::string& dataPath);
		/** @brief Get the data path */
		GLORY_RUNTIME_API std::string_view GetDataPath();

	private:
		/** @brief Callback when the rendering of a frame starts */
		void GraphicsThreadBeginRender() {}
		/** @brief Callback when the rendering of a frame ends */
		void GraphicsThreadEndRender();

	private:
		friend class GraphicsThread;
		Engine* m_pEngine;
		RendererModule* m_pRenderer;
		GraphicsModule* m_pGraphics;
		WindowModule* m_pWindows;
		std::unique_ptr<RuntimeSceneManager> m_SceneManager;
		std::unique_ptr<RuntimeMaterialManager> m_MaterialManager;
		std::unique_ptr<RuntimeShaderManager> m_ShaderManager;
		std::vector<std::filesystem::path> m_AppendedAssetDatabases;
		std::string m_DataPath;
	};
}
