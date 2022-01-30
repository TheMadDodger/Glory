#pragma once
#include "Commands.h"
#include <thread>
#include <queue>
#include <functional>

namespace Glory
{
#if _DEBUG
	class DebugConsoleInput;
#endif
	class Console
	{
	public:
		static void RegisterCommand(BaseConsoleCommand* pCommand);
		static void QueueCommand(const std::string& command);
		static void ExecuteCommand(const std::string& command, bool addToHistory = true);
		static void WriteLine(const std::string& line, bool addTimestamp = true);
		static void ForEachCommandInHistory(std::function<void(const std::string&)> callback);

		static void Update();

	private:
		Console();
		virtual ~Console();

	private:
		static void Initialize();
		static void Cleanup();
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
		static const int MAX_HISTORY_SIZE = 10;
		static const int MAX_CONSOLE_SIZE = 2;
		static int m_CommandHistoryInsertIndex;
		static int m_CurrentCommandHistorySize;
		static int m_ConsoleInsertIndex;
		static int m_CurrentConsoleSize;

		std::vector<std::string> m_CommandHistory;
		std::vector<std::string> m_ConsoleLines;
		std::queue<std::string> m_CommandQueue;
#if _DEBUG
		static DebugConsoleInput* m_pDebugConsole;
#endif
		static bool m_Writing;
		static bool m_Reading;
		static Console* m_pInstance;
	};
}