#include "DebugConsoleInput.h"
#include "Commands.h"
#include <algorithm>
#include <iostream>
#include "Console.h"
//#include <WinBase.h>
//#include <consoleapi2.h>

namespace Glory
{
#ifdef _DEBUG
	inline int ConsoleThreadFunc(void* pConsoleObject)
	{
		DebugConsoleInput* pConsole = (DebugConsoleInput*)pConsoleObject;

		while (pConsole->Running())
		{
			pConsole->WaitForInput();
		}

		return 0;
	}

	DebugConsoleInput::DebugConsoleInput() : m_Running(false), m_pConsoleThread() {}

	DebugConsoleInput::~DebugConsoleInput() {}

	void DebugConsoleInput::Initialize()
	{
		m_Running = true;
		m_pConsoleThread = std::thread(ConsoleThreadFunc, this); //SDL_CreateThread(ConsoleThreadFunc, "ConsoleThread", this);
		m_pConsoleThread.detach();
	}

	void DebugConsoleInput::WaitForInput()
	{
		std::string line = "";
		std::getline(std::cin, line);

		Console::QueueCommand(line);
	}

	bool DebugConsoleInput::Running()
	{
		return m_Running;
	}

	void DebugConsoleInput::Stop()
	{
		if (m_pConsoleThread.joinable()) m_pConsoleThread.join();
		m_Running = false;
	}

	void DebugConsoleInput::SetConsoleColor(unsigned int color)
	{
		//HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
		//SetConsoleTextAttribute(hStdOut, color);
	}

	void DebugConsoleInput::ResetConsoleColor()
	{
		//HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
		//SetConsoleTextAttribute(hStdOut, 15);
	}

	void DebugConsoleInput::Write(const std::string& line)
	{
		std::cout << line << std::endl;
	}
#endif // _DEBUG
}