#pragma once
#include "Visibility.h"

#include <memory>
#include <vector>

namespace std::filesystem
{
	class path;
}

namespace Glory
{
	class Engine;
	class RuntimeMaterialManager;
	class RuntimeShaderManager;
	class RendererModule;
	class GraphicsModule;

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
		/** @brief Load an asset group at a path */
		GLORY_RUNTIME_API void LoadAssetGroup(const std::filesystem::path& path);
		/** @brief Load a shader pack at a path */
		GLORY_RUNTIME_API void LoadShaderPack(const std::filesystem::path& path);
		/** @brief Load a scene, its assets, shaders and asset database at a path */
		GLORY_RUNTIME_API void LoadScene(const std::filesystem::path& path);
		/** @brief Load a scene only at a path */
		GLORY_RUNTIME_API void LoadSceneOnly(const std::filesystem::path& path);
		/** @brief Get the engine attached to this runtime */
		GLORY_RUNTIME_API Engine* GetEngine();

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
		std::unique_ptr<RuntimeMaterialManager> m_MaterialManager;
		std::unique_ptr<RuntimeShaderManager> m_ShaderManager;
		std::vector<std::filesystem::path> m_AppendedAssetDatabases;
	};
}
