#pragma once
#include <memory>
#include <string_view>

namespace Glory
{
	class Debug;
	class Console;
	class Engine;
	class GloryRuntime;
	class EngineLoader;

	class GloryRuntimeApplication
	{
	public:
		GloryRuntimeApplication();
		GloryRuntimeApplication(const std::string_view appName);
		~GloryRuntimeApplication();

		void SetName(const std::string_view appName);
		void Initialize(int argc, char* argv[]);
		void Run();
		void Cleanup();

		GloryRuntime& Runtime();

	private:
		std::string_view m_AppName;
		std::unique_ptr<Debug> m_Debug;
		std::unique_ptr<Console> m_Console;
		std::unique_ptr<EngineLoader> m_EngineLoader;
		std::unique_ptr<Engine> m_Engine;
		std::unique_ptr<GloryRuntime> m_Runtime;
	};
}
