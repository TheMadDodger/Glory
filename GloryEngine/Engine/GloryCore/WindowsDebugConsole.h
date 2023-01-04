#pragma once
#include <string>
#include <vector>
#include <thread>
#include "IConsole.h"

namespace Glory
{
	class BaseConsoleCommand;

	class WindowsDebugConsole : public IConsole
	{
	public:
		bool Running();
		void WaitForInput();

	private:
		WindowsDebugConsole();
		virtual ~WindowsDebugConsole();

		void Initialize() override;
		void OnConsoleClose() override;

		void SetNextColor(const glm::vec4& color) override;
		void ResetNextColor() override;
		void Write(const std::string& line) override;

	private:
		friend class Console;
		friend class Debug;
		std::thread m_pConsoleThread;
		bool m_Running;
	};
}