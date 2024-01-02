#pragma once
#include "Commands.h"
#include "IConsole.h"

#include <thread>
#include <queue>
#include <functional>
#include <mutex>

namespace Glory
{
	class Engine;

	class Console
	{
	public:
		Console();
		virtual ~Console();

		void Initialize();
		void Cleanup();

		void RegisterCommand(BaseConsoleCommand* pCommand);
		void QueueCommand(const std::string& command);
		void ExecuteCommand(const std::string& command, bool addToHistory = true);
		void WriteLine(const std::string& line, bool addTimestamp = true);
		void ForEachCommandInHistory(std::function<void(const std::string&)> callback);

		void SetNextColor(const glm::vec4& color);
		void Update();

		template<class T>
		void RegisterConsole()
		{
			IConsole* pConsole = new T();
			m_pConsoles.push_back(pConsole);
			pConsole->Initialize();
		}

		void RegisterConsole(IConsole* pConsole)
		{
			m_pConsoles.push_back(pConsole);
			pConsole->Initialize();
		}

		void RemoveConsole(IConsole* pConsole)
		{
			auto it = std::find(m_pConsoles.begin(), m_pConsoles.end(), pConsole);
			if (it == m_pConsoles.end()) return;
			m_pConsoles.erase(it);
		}

	private:
		bool PrintHistory();

		void AddCommandToHistory(const std::string& command);
		void AddLineToConsole(const std::string& line);

		void SeperateArguments(const std::string& input, std::string& command, std::vector<std::string>& args);
		BaseConsoleCommand* GetCommand(const std::string& command);

		std::string TimeStamp();

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

		std::mutex m_Lock;

		std::vector<std::string> m_CommandHistory;
		std::vector<std::string> m_ConsoleLines;
		std::queue<std::string> m_CommandQueue;

		static const int MAX_HISTORY_SIZE = 10;
		static const int MAX_CONSOLE_SIZE = 2;
	};
}