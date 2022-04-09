#include "Debug.h"
#include "Console.h"
#include "Game.h"

namespace Glory
{
	ThreadedVector<std::string> Debug::m_LoggedKeys;

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
		//Console::SetConsoleColor(15);
		Console::SetNextColor({1.f, 1.f, 1.f, 1.0f});
		std::string finalMessage = "Info:	" + message;
		Console::WriteLine(finalMessage, bIncludeTimeStamp);
	}

	void Debug::LogNotice(const std::string& message, bool bIncludeTimeStamp)
	{
		//Console::SetConsoleColor(8);
		Console::SetNextColor({ 0.5f, 0.5f, 0.5f, 1.0f });
		std::string finalMessage = "Notice:	" + message;
		Console::WriteLine(finalMessage, bIncludeTimeStamp);

	}

	void Debug::LogWarning(const std::string& message, bool bIncludeTimeStamp)
	{
		//Console::SetConsoleColor(14);
		Console::SetNextColor({ 1.f, 0.918f, 0.0f, 1.0f });
		std::string finalMessage = "WARNING:	" + message;
		Console::WriteLine(finalMessage, bIncludeTimeStamp);
	}

	void Debug::LogError(const std::string& message, bool bIncludeTimeStamp)
	{
		//Console::SetConsoleColor(12);
		Console::SetNextColor({ 1.f, 0.0f, 0.0f, 1.0f });
		std::string finalMessage = "ERROR:	" + message;
		Console::WriteLine(finalMessage, bIncludeTimeStamp);
	}

	void Debug::LogFatalError(const std::string& message, bool bIncludeTimeStamp)
	{
		//Console::SetConsoleColor(12);
		Console::SetNextColor({ 1.f, 0.0f, 0.0f, 1.0f });
		std::string finalMessage = "FATAL ERROR:	" + message;
		Console::WriteLine(finalMessage, bIncludeTimeStamp);

		// Open message box
		Game::GetGame().GetEngine()->GetWindowModule()->OpenMessageBox("FATAL ERROR: " + message);
		Game::GetGame().Quit();
	}

	void Debug::LogOnce(const std::string& key, const std::string& message, const LogLevel& logLevel, bool bIncludeTimeStamp)
	{
		if (m_LoggedKeys.Contains(key)) return;
		m_LoggedKeys.push_back(key);
		Log(message, logLevel, bIncludeTimeStamp);
	}

	Debug::Debug()
	{
	}

	Debug::~Debug()
	{
	}
}