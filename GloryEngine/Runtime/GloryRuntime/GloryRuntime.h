#pragma once
#include "Visibility.h"

#include <memory>

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
		/** @brief Run the runtime */
		GLORY_RUNTIME_API void Run();

	private:
		Engine* m_pEngine;
		std::unique_ptr<RuntimeMaterialManager> m_MaterialManager;
		std::unique_ptr<RuntimeShaderManager> m_ShaderManager;
	};
}
