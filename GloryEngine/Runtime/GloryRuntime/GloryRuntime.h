#pragma once
#include "Visibility.h"

#include <memory>

namespace std::filesystem
{
	class path;
}

namespace Glory
{
	class Engine;
	class RuntimeMaterialManager;
	class RuntimeShaderManager;

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
		/** @brief Load a scene at a path */
		GLORY_RUNTIME_API void LoadScene(const std::filesystem::path& path);

	private:
		Engine* m_pEngine;
		std::unique_ptr<RuntimeMaterialManager> m_MaterialManager;
		std::unique_ptr<RuntimeShaderManager> m_ShaderManager;
	};
}
