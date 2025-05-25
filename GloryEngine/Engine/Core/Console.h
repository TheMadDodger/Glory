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

	/** @brief Console variable data */
	struct CVar
	{
		std::string m_Name;
		std::string m_Description;
		float m_Value;

		/** @brief CVar flags
		 * @todo: These dont do anything yet */
		enum Flags
		{
			/** @brief No flags */
			None = 0,
			/** @brief Write out value on save */
			Save = 1 << 0,
			/** @brief Changing this value is only allowed when cheats are enabled */
			Cheat = 1 << 1,
			/** @brief Enables cheats */
			CheatsOn = 1 << 2,
			/** @brief Server variable */
			Server = 1 << 3,
			/** @brief Requires authentication */
			RequiresAuth = 1 << 4
		};
		Flags m_Flags;
	};

	class Console
	{
	public:
		Console();
		virtual ~Console();

		void Initialize();
		void Cleanup();

		void RegisterCommand(BaseConsoleCommand* pCommand);
		void RegisterCVar(CVar&& var);
		void QueueCommand(const std::string& command);
		void ExecuteCommand(const std::string& command, bool addToHistory = true);
		void WriteLine(const std::string& line, bool addTimestamp = true);
		void ForEachCommandInHistory(std::function<void(const std::string&)> callback);

		void SetNextColor(const glm::vec4& color);
		void Update();

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

		size_t LineCount() const;
		std::string_view Line(size_t index) const;
		const glm::vec4& LineColor(size_t index) const;

		size_t HistoryCount() const;
		const std::string& History(size_t rewwindIndex) const;

	private:
		bool PrintHistory();

		void AddCommandToHistory(const std::string& command);
		void AddLineToConsole(const std::string& line);

		void SeperateArguments(const std::string& input, std::string& command, std::vector<std::string>& args);
		BaseConsoleCommand* GetCommand(const std::string& command);

		std::string TimeStamp();

		CVar* FindCVar(std::string_view name);
		void ExecuteCVarCommand(CVar& cvar, std::vector<std::string>& args);

	private:
		friend class Engine;
		friend class Debug;
		friend class GloryContext;
		std::vector<BaseConsoleCommand*> m_pCommands;
		std::vector<IConsole*> m_pConsoles;
		bool m_Writing = false;
		bool m_Reading = false;

		std::mutex m_Lock;

		std::vector<std::string> m_CommandHistory;
		std::vector<std::string> m_ConsoleLines;
		std::vector<glm::vec4> m_ConsoleLineColors;
		glm::vec4 m_CurrentColor;
		std::queue<std::string> m_CommandQueue;
		std::vector<CVar> m_CVars;
	};
}