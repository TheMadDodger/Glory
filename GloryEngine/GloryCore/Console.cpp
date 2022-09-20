#include "Console.h"
#include "Commands.h"
#include <iostream>
#include <iomanip>

namespace Glory
{
	//std::vector<BaseConsoleCommand*> Console::m_pCommands = std::vector<BaseConsoleCommand*>();
	//std::vector<IConsole*> Console::m_pConsoles = std::vector<IConsole*>();

	//bool Console::m_Writing = false;
	//bool Console::m_Reading = false;
	//int Console::m_CommandHistoryInsertIndex = -1;
	//int Console::m_ConsoleInsertIndex = -1;
	//int Console::m_CurrentCommandHistorySize = 0;
	//int Console::m_CurrentConsoleSize = 0;

	//Console* Console::m_pInstance = nullptr;

	void Console::Initialize()
	{
		//if (CONSOLE_INSTANCE->m_pInstance != nullptr) return;
		//CONSOLE_INSTANCE->m_pInstance = new Console();
		RegisterCommand(new ConsoleCommand("printhistory", Console::PrintHistory));
	}

	void Console::Cleanup()
	{
		for (auto pCommand : CONSOLE_INSTANCE->m_pCommands)
		{
			delete pCommand;
		}
		CONSOLE_INSTANCE->m_pCommands.clear();

		Parser::Destroy();

		for (size_t i = 0; i < CONSOLE_INSTANCE->m_pConsoles.size(); i++)
		{
			CONSOLE_INSTANCE->m_pConsoles[i]->OnConsoleClose();
			delete CONSOLE_INSTANCE->m_pConsoles[i];
		}
		CONSOLE_INSTANCE->m_pConsoles.clear();
	}

	void Console::Update()
	{
		if (CONSOLE_INSTANCE->m_Writing) return;
		if (CONSOLE_INSTANCE->m_CommandQueue.empty()) return;

		CONSOLE_INSTANCE->m_Reading = true;
		const std::string& command = CONSOLE_INSTANCE->m_CommandQueue.front();
		ExecuteCommand(command);
		CONSOLE_INSTANCE->m_CommandQueue.pop();
		CONSOLE_INSTANCE->m_Reading = false;
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
		++CONSOLE_INSTANCE->m_CommandHistoryInsertIndex;
		if (CONSOLE_INSTANCE->m_CommandHistoryInsertIndex >= MAX_HISTORY_SIZE) CONSOLE_INSTANCE->m_CommandHistoryInsertIndex = 0;
		if (CONSOLE_INSTANCE->m_CurrentCommandHistorySize < MAX_HISTORY_SIZE) ++CONSOLE_INSTANCE->m_CurrentCommandHistorySize;
		CONSOLE_INSTANCE->m_CommandHistory[CONSOLE_INSTANCE->m_CommandHistoryInsertIndex] = command;
	}

	void Console::AddLineToConsole(const std::string& line)
	{
		++CONSOLE_INSTANCE->m_ConsoleInsertIndex;
		if (CONSOLE_INSTANCE->m_ConsoleInsertIndex >= MAX_CONSOLE_SIZE) CONSOLE_INSTANCE->m_ConsoleInsertIndex = 0;
		if (CONSOLE_INSTANCE->m_CurrentConsoleSize < MAX_CONSOLE_SIZE) ++CONSOLE_INSTANCE->m_CurrentConsoleSize;
		CONSOLE_INSTANCE->m_ConsoleLines[CONSOLE_INSTANCE->m_ConsoleInsertIndex] = line;
	}

	void Console::RegisterCommand(BaseConsoleCommand* pCommand)
	{
		CONSOLE_INSTANCE->m_pCommands.push_back(pCommand);
	}

	void Console::QueueCommand(const std::string& command)
	{
		// If m_Reading is true this function is called from another thread, this thread will need to wait untill we are done reading on the main thread!
		while (CONSOLE_INSTANCE->m_Reading)
		{
			// Do nothing
		}

		CONSOLE_INSTANCE->m_Writing = true;
		CONSOLE_INSTANCE->m_CommandQueue.push(command);
		CONSOLE_INSTANCE->m_Writing = false;
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
				Debug::LogNotice(">>>> Could not execute command", false);
		}
		else
			Debug::LogNotice(">>>> Unknown Command", false);
	}

	void Console::WriteLine(const std::string& line, bool addTimestamp)
	{
		std::string finalLine = line;
		if (addTimestamp) finalLine = TimeStamp() + finalLine;
		AddLineToConsole(finalLine);

		for (size_t i = 0; i < CONSOLE_INSTANCE->m_pConsoles.size(); i++)
		{
			CONSOLE_INSTANCE->m_pConsoles[i]->Write(finalLine);
		}
	}

	void Console::ForEachCommandInHistory(std::function<void(const std::string&)> callback)
	{
		if (CONSOLE_INSTANCE->m_CurrentCommandHistorySize < MAX_HISTORY_SIZE)
		{
			for (int i = CONSOLE_INSTANCE->m_CommandHistoryInsertIndex; i >= 0; --i)
			{
				const std::string& command = CONSOLE_INSTANCE->m_CommandHistory[(size_t)i];
				callback(command);
			}
			return;
		}

		int currentIndex = CONSOLE_INSTANCE->m_CommandHistoryInsertIndex;
		for (size_t i = 0; i < MAX_HISTORY_SIZE; i++)
		{
			const std::string& command = CONSOLE_INSTANCE->m_CommandHistory[(size_t)currentIndex];
			callback(command);

			--currentIndex;
			if (currentIndex < 0) currentIndex = MAX_HISTORY_SIZE - 1;
		}
	}

	void Console::SetNextColor(const glm::vec4& color)
	{
		for (size_t i = 0; i < CONSOLE_INSTANCE->m_pConsoles.size(); i++)
		{
			CONSOLE_INSTANCE->m_pConsoles[i]->SetNextColor(color);
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
		auto it = std::find_if(CONSOLE_INSTANCE->m_pCommands.begin(), CONSOLE_INSTANCE->m_pCommands.end(), [&](BaseConsoleCommand* pCommand)
		{
			if (pCommand->GetCommand() == command) return true;
			return false;
		});

		if (it == CONSOLE_INSTANCE->m_pCommands.end()) return nullptr;
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

	Console::Console() : m_CommandHistory(std::vector<std::string>(MAX_HISTORY_SIZE)), m_ConsoleLines(std::vector<std::string>(MAX_CONSOLE_SIZE)) {}

	Console::~Console()
	{
		m_CommandHistory.clear();
		m_ConsoleLines.clear();
	}
}
