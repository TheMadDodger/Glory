#pragma once
#include <string>

namespace Glory
{
	class Debug
	{
	public:
		enum class LogLevel
		{
			/// <summary>
			/// Normal debug info for debugging.
			/// </summary>
			Info,
			/// <summary>
			/// A notice for things that still attention, bug fixes, changes, additions etc.
			/// </summary>
			Notice,
			/// <summary>
			/// A warning will be displayed in yellow.
			/// </summary>
			Warning,
			/// <summary>
			/// An error will be displayed in red.
			/// </summary>
			Error,
			/// <summary>
			/// A fatal error will display the error in a seperate window and then close the game.
			/// </summary>
			FatalError,
		};

	public:
		static void Log(const std::string& message, const LogLevel& logLevel, bool bIncludeTimeStamp = true);
		static void LogInfo(const std::string& message, bool bIncludeTimeStamp = true);
		static void LogNotice(const std::string& message, bool bIncludeTimeStamp = true);
		static void LogWarning(const std::string& message, bool bIncludeTimeStamp = true);
		static void LogError(const std::string& message, bool bIncludeTimeStamp = true);
		static void LogFatalError(const std::string& message, bool bIncludeTimeStamp = true);

	private:
		Debug();
		virtual ~Debug();
	};
}