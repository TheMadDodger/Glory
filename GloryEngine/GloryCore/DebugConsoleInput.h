#pragma once
#include <string>
#include <vector>
#include <thread>

namespace Glory
{
#ifdef _DEBUG
	class BaseConsoleCommand;

	class DebugConsoleInput
	{
	public:
		bool Running();
		void WaitForInput();

	private:
		DebugConsoleInput();
		virtual ~DebugConsoleInput();

		void Initialize();
		void Stop();

		void SetConsoleColor(unsigned int color);
		void ResetConsoleColor();
		void Write(const std::string& line);

	private:
		friend class Console;
		friend class Debug;
		std::thread m_pConsoleThread;
		bool m_Running;
	};
#endif // _DEBUG
}