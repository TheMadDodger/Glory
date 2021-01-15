#include "Debug.h"
#include "Console.h"
#include "DebugConsoleInput.h"
#include "Game.h"

namespace Glory
{
	void Debug::Log(const std::string& message, const LogLevel& logLevel, bool bIncludeTimeStamp)
	{
		switch (logLevel)
		{
		case LogLevel::Info:
			LogInfo(message, bIncludeTimeStamp);
			break;

		case LogLevel::Notice:
			LogNotice(message, bIncludeTimeStamp);
			break;

		case LogLevel::Warning:
			LogWarning(message, bIncludeTimeStamp);
			break;

		case LogLevel::Error:
			LogError(message, bIncludeTimeStamp);
			break;

		case LogLevel::FatalError:
			LogFatalError(message, bIncludeTimeStamp);
			break;
		default:
			break;
		}
	}

	void Debug::LogInfo(const std::string& message, bool bIncludeTimeStamp)
	{
#if _DEBUG
		Console::m_pDebugConsole->SetConsoleColor(15);
#endif
		std::string finalMessage = "Info:	" + message;
		Console::WriteLine(finalMessage, bIncludeTimeStamp);
#if _DEBUG
		Console::m_pDebugConsole->ResetConsoleColor();
#endif
	}

	void Debug::LogNotice(const std::string& message, bool bIncludeTimeStamp)
	{
#if _DEBUG
		Console::m_pDebugConsole->SetConsoleColor(8);
#endif
		std::string finalMessage = "Notice:	" + message;
		Console::WriteLine(finalMessage, bIncludeTimeStamp);
#if _DEBUG
		Console::m_pDebugConsole->ResetConsoleColor();
#endif

	}

	void Debug::LogWarning(const std::string& message, bool bIncludeTimeStamp)
	{
#if _DEBUG
		Console::m_pDebugConsole->SetConsoleColor(14);
#endif
		std::string finalMessage = "WARNING:	" + message;
		Console::WriteLine(finalMessage, bIncludeTimeStamp);
#if _DEBUG
		Console::m_pDebugConsole->ResetConsoleColor();
#endif

	}

	void Debug::LogError(const std::string& message, bool bIncludeTimeStamp)
	{
#if _DEBUG
		Console::m_pDebugConsole->SetConsoleColor(12);
#endif
		std::string finalMessage = "ERROR:	" + message;
		Console::WriteLine(finalMessage, bIncludeTimeStamp);
#if _DEBUG
		Console::m_pDebugConsole->ResetConsoleColor();
#endif
	}

	void Debug::LogFatalError(const std::string& message, bool bIncludeTimeStamp)
	{
#if _DEBUG
		Console::m_pDebugConsole->SetConsoleColor(12);
#endif
		std::string finalMessage = "FATAL ERROR:	" + message;
		Console::WriteLine(finalMessage, bIncludeTimeStamp);
#if _DEBUG
		Console::m_pDebugConsole->ResetConsoleColor();
#endif

		// Open message box
		Game::GetGame().GetEngine()->GetWindowModule()->OpenMessageBox("FATAL ERROR: " + message);
		Game::GetGame().Quit();
	}

	Debug::Debug()
	{
	}

	Debug::~Debug()
	{
	}
}