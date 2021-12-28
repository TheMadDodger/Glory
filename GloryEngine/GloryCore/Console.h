#pragma once
#include "Commands.h"
#include "IConsole.h"
#include <thread>
#include <queue>
#include <functional>

namespace Glory
{
	class Console
	{
	public:
		static void RegisterCommand(BaseConsoleCommand* pCommand);
		static void QueueCommand(const std::string& command);
		static void ExecuteCommand(const std::string& command, bool addToHistory = true);
		static void WriteLine(const std::string& line, bool addTimestamp = true);
		static void ForEachCommandInHistory(std::function<void(const std::string&)> callback);

		static void SetNextColor(const glm::vec4& color);

		template<class T>
		static void RegisterConsole()
		{
			IConsole* pConsole = new T();
			m_pConsoles.push_back(pConsole);
			pConsole->Initialize();
		}

		static void RegisterConsole(IConsole* pConsole)
		{
			m_pConsoles.push_back(pConsole);
			pConsole->Initialize();
		}

		static void RemoveConsole(IConsole* pConsole)
		{
			auto it = std::find(m_pConsoles.begin(), m_pConsoles.end(), pConsole);
			if (it == m_pConsoles.end()) return;
			m_pConsoles.erase(it);
		}

	private:
		Console();
		virtual ~Console();

	private:
		static void Initialize();
		static void Cleanup();
		static void Update();
		static bool PrintHistory();

		static void AddCommandToHistory(const std::string& command);
		static void AddLineToConsole(const std::string& line);

		static void SeperateArguments(const std::string& input, std::string& command, std::vector<std::string>& args);
		static BaseConsoleCommand* GetCommand(const std::string& command);

		static std::string TimeStamp();

	private:
		friend class Engine;
		friend class Debug;
		static std::vector<BaseConsoleCommand*> m_pCommands;
		static std::vector<IConsole*> m_pConsoles;
		static const int MAX_HISTORY_SIZE = 10;
		static const int MAX_CONSOLE_SIZE = 2;
		static int m_CommandHistoryInsertIndex;
		static int m_CurrentCommandHistorySize;
		static int m_ConsoleInsertIndex;
		static int m_CurrentConsoleSize;
		static bool m_Writing;
		static bool m_Reading;
		static Console* m_pInstance;

		std::vector<std::string> m_CommandHistory;
		std::vector<std::string> m_ConsoleLines;
		std::queue<std::string> m_CommandQueue;
	};
}