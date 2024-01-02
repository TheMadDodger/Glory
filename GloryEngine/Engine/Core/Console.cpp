#include "Console.h"
#include "Commands.h"
#include <iostream>
#include <iomanip>

namespace Glory
{
	void Console::Initialize()
	{
		//if (m_pInstance != nullptr) return;
		//m_pInstance = new Console();
		RegisterCommand(new ConsoleCommand("printhistory", [this]() { return PrintHistory(); }));
	}

	void Console::Cleanup()
	{
		
	}

	void Console::Update()
	{
		if (m_Writing) return;
		if (m_CommandQueue.empty()) return;

		m_Reading = true;
		const std::string& command = m_CommandQueue.front();
		ExecuteCommand(command);
		m_CommandQueue.pop();
		m_Reading = false;
	}

	bool Console::PrintHistory()
	{
		ForEachCommandInHistory([=](const std::string& command)
		{
			WriteLine(command);
		});
		return true;
	}

	void Console::AddCommandToHistory(const std::string& command)
	{
		++m_CommandHistoryInsertIndex;
		if (m_CommandHistoryInsertIndex >= MAX_HISTORY_SIZE) m_CommandHistoryInsertIndex = 0;
		if (m_CurrentCommandHistorySize < MAX_HISTORY_SIZE) ++m_CurrentCommandHistorySize;
		m_CommandHistory[m_CommandHistoryInsertIndex] = command;
	}

	void Console::AddLineToConsole(const std::string& line)
	{
		++m_ConsoleInsertIndex;
		if (m_ConsoleInsertIndex >= MAX_CONSOLE_SIZE) m_ConsoleInsertIndex = 0;
		if (m_CurrentConsoleSize < MAX_CONSOLE_SIZE) ++m_CurrentConsoleSize;
		m_ConsoleLines[m_ConsoleInsertIndex] = line;
	}

	void Console::RegisterCommand(BaseConsoleCommand* pCommand)
	{
		m_pCommands.push_back(pCommand);
	}

	void Console::QueueCommand(const std::string& command)
	{
		// If m_Reading is true this function is called from another thread, this thread will need to wait untill we are done reading on the main thread!
		while (m_Reading)
		{
			// Do nothing
		}

		m_Writing = true;
		m_CommandQueue.push(command);
		m_Writing = false;
	}

	void Console::ExecuteCommand(const std::string& command, bool addToHistory)
	{
		if (command == "") return;

		if (addToHistory) AddCommandToHistory(command);
		WriteLine(command, false);

		std::string commandText;
		std::vector<std::string> args;
		SeperateArguments(command, commandText, args);
		BaseConsoleCommand* pCommand = GetCommand(commandText);

		if (pCommand != nullptr)
		{
			if (pCommand->RootExecuteCommand(args)) return;
				WriteLine(">>>> Could not execute command", false);
		}
		else
			WriteLine(">>>> Unknown Command", false);
	}

	void Console::WriteLine(const std::string& line, bool addTimestamp)
	{
		std::unique_lock lock{m_Lock};

		std::string finalLine = line;
		if (addTimestamp) finalLine = TimeStamp() + finalLine;
		AddLineToConsole(finalLine);

		for (size_t i = 0; i < m_pConsoles.size(); i++)
		{
			m_pConsoles[i]->Write(finalLine);
		}
	}

	void Console::ForEachCommandInHistory(std::function<void(const std::string&)> callback)
	{
		if (m_CurrentCommandHistorySize < MAX_HISTORY_SIZE)
		{
			for (int i = m_CommandHistoryInsertIndex; i >= 0; --i)
			{
				const std::string& command = m_CommandHistory[(size_t)i];
				callback(command);
			}
			return;
		}

		int currentIndex = m_CommandHistoryInsertIndex;
		for (size_t i = 0; i < MAX_HISTORY_SIZE; i++)
		{
			const std::string& command = m_CommandHistory[(size_t)currentIndex];
			callback(command);

			--currentIndex;
			if (currentIndex < 0) currentIndex = MAX_HISTORY_SIZE - 1;
		}
	}

	void Console::SetNextColor(const glm::vec4& color)
	{
		for (size_t i = 0; i < m_pConsoles.size(); i++)
		{
			m_pConsoles[i]->SetNextColor(color);
		}
	}

	void Console::SeperateArguments(const std::string& input, std::string& command, std::vector<std::string>& args)
	{
		command = "";
		args.clear();

		int argNumber = 0;
		std::string currentArg = "";
		for (size_t i = 0; i < input.size(); i++)
		{
			char c = input[i];
			if (c != ' ')
			{
				currentArg.push_back(c);

				if (i == input.size() - 1)
				{
					if (argNumber == 0)
					{
						command = currentArg;
						currentArg = "";
						++argNumber;
						continue;
					}
					args.push_back(currentArg);
					currentArg = "";
					++argNumber;
				}
			}
			else
			{
				if (argNumber == 0)
				{
					command = currentArg;
					currentArg = "";
					++argNumber;
					continue;
				}
				args.push_back(currentArg);
				currentArg = "";
				++argNumber;
			}
		}
	}

	BaseConsoleCommand* Console::GetCommand(const std::string& command)
	{
		auto it = std::find_if(m_pCommands.begin(), m_pCommands.end(), [&](BaseConsoleCommand* pCommand)
		{
			if (pCommand->GetCommand() == command) return true;
			return false;
		});

		if (it == m_pCommands.end()) return nullptr;
		BaseConsoleCommand* pCommand = *it;
		return pCommand;
	}

	std::string Console::TimeStamp()
	{
		auto t = time(nullptr);
		tm tM;
		localtime_s(&tM, &t);
		std::stringstream sstream;
		sstream << "[" << std::put_time(&tM, "%d-%m-%Y %H-%M-%S") << "] ";
		return sstream.str();
	}

	Console::Console():
		m_CommandHistory(std::vector<std::string>(MAX_HISTORY_SIZE)),
		m_ConsoleLines(std::vector<std::string>(MAX_CONSOLE_SIZE))
	{}

	Console::~Console()
	{
		for (auto pCommand : m_pCommands)
		{
			delete pCommand;
		}
		m_pCommands.clear();

		Parser::Destroy();

		for (size_t i = 0; i < m_pConsoles.size(); i++)
		{
			m_pConsoles[i]->OnConsoleClose();
			delete m_pConsoles[i];
		}
		m_pConsoles.clear();

		m_CommandHistory.clear();
		m_ConsoleLines.clear();
	}
}
