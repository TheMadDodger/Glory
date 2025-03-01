#pragma once
#include "IConsole.h"

#include <string>
#include <vector>
#include <thread>

namespace Glory
{
	class Console;
	class BaseConsoleCommand;

	class WindowsDebugConsole : public IConsole
	{
	public:
		WindowsDebugConsole(Console* pConsole);
		virtual ~WindowsDebugConsole();

		bool Running();
		void WaitForInput();

	private:
		void Initialize() override;
		void OnConsoleClose() override;

		void SetNextColor(const glm::vec4& color) override;
		void ResetNextColor() override;
		void Write(const std::string& line) override;

	private:
		friend class Console;
		friend class Debug;
		Console* m_pConsole;
		std::thread m_pConsoleThread;
		bool m_Running;
	};
}