#pragma once
#include "Commands.h"
#include "IConsole.h"
#include "GloryContext.h"
#include <thread>
#include <queue>
#include <functional>

#define CONSOLE_INSTANCE GloryContext::GetContext()->GetConsole()

namespace Glory
{
	class Console
	{
	public:
		static void Initialize();
		static void Cleanup();

		static void RegisterCommand(BaseConsoleCommand* pCommand);
		static void QueueCommand(const std::string& command);
		static void ExecuteCommand(const std::string& command, bool addToHistory = true);
		static void WriteLine(const std::string& line, bool addTimestamp = true);
		static void ForEachCommandInHistory(std::function<void(const std::string&)> callback);

		static void SetNextColor(const glm::vec4& color);
		static void Update();

		template<class T>
		static void RegisterConsole()
		{
			IConsole* pConsole = new T();
			CONSOLE_INSTANCE->m_pConsoles.push_back(pConsole);
			pConsole->Initialize();
		}

		static void RegisterConsole(IConsole* pConsole)
		{
			CONSOLE_INSTANCE->m_pConsoles.push_back(pConsole);
			pConsole->Initialize();
		}

		static void RemoveConsole(IConsole* pConsole)
		{
			auto it = std::find(CONSOLE_INSTANCE->m_pConsoles.begin(), CONSOLE_INSTANCE->m_pConsoles.end(), pConsole);
			if (it == CONSOLE_INSTANCE->m_pConsoles.end()) return;
			CONSOLE_INSTANCE->m_pConsoles.erase(it);
		}

	private:
		Console();
		virtual ~Console();

	private:
		static bool PrintHistory();

		static void AddCommandToHistory(const std::string& command);
		static void AddLineToConsole(const std::string& line);

		static void SeperateArguments(const std::string& input, std::string& command, std::vector<std::string>& args);
		static BaseConsoleCommand* GetCommand(const std::string& command);

		static std::string TimeStamp();

	private:
		friend class Engine;
		friend class Debug;
		friend class GloryContext;
		std::vector<BaseConsoleCommand*> m_pCommands;
		std::vector<IConsole*> m_pConsoles;
		int m_CommandHistoryInsertIndex = -1;
		int m_CurrentCommandHistorySize = 0;
		int m_ConsoleInsertIndex = -1;
		int m_CurrentConsoleSize = 0;
		bool m_Writing = false;
		bool m_Reading = false;

		std::vector<std::string> m_CommandHistory;
		std::vector<std::string> m_ConsoleLines;
		std::queue<std::string> m_CommandQueue;

		static const int MAX_HISTORY_SIZE = 10;
		static const int MAX_CONSOLE_SIZE = 2;
	};
}