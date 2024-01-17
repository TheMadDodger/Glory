#include "WindowsDebugConsole.h"
#include "Commands.h"
#include "Console.h"

#include <algorithm>
#include <iostream>
//#include <consoleapi2.h>
//#include <WinBase.h>

namespace Glory
{
	inline int ConsoleThreadFunc(void* pConsoleObject)
	{
		WindowsDebugConsole* pConsole = (WindowsDebugConsole*)pConsoleObject;

		while (pConsole->Running())
		{
			pConsole->WaitForInput();
		}

		return 0;
	}

	WindowsDebugConsole::WindowsDebugConsole(Console* pConsole):
		m_pConsole(pConsole), m_Running(false), m_pConsoleThread() {}

	WindowsDebugConsole::~WindowsDebugConsole() {}

	void WindowsDebugConsole::Initialize()
	{
		m_Running = true;
		m_pConsoleThread = std::thread(ConsoleThreadFunc, this); //SDL_CreateThread(ConsoleThreadFunc, "ConsoleThread", this);
		m_pConsoleThread.detach();
	}

	void WindowsDebugConsole::WaitForInput()
	{
		std::string line = "";
		std::getline(std::cin, line);
		m_pConsole->QueueCommand(line);
	}

	bool WindowsDebugConsole::Running()
	{
		return m_Running;
	}

	void WindowsDebugConsole::OnConsoleClose()
	{
		if (m_pConsoleThread.joinable()) m_pConsoleThread.join();
		m_Running = false;
	}

	void WindowsDebugConsole::SetNextColor(const glm::vec4& color)
	{
		//HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
		//SetConsoleTextAttribute(hStdOut, color);
	}

	void WindowsDebugConsole::ResetNextColor()
	{
		//HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
		//SetConsoleTextAttribute(hStdOut, 15);
	}

	void WindowsDebugConsole::Write(const std::string& line)
	{
		std::cout << line << std::endl;
	}
}