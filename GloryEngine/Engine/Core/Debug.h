#pragma once
#include "Glory.h"

#include <string>
#include <vector>
#include <mutex>

#ifndef GLORY_NO_DEBUG_LINES

#include <glm/glm.hpp>

#endif

namespace Glory
{
	class RendererModule;
	class Console;
	class GameTime;

	class Debug
	{
	public:
		Debug(Console* pConsole) : m_pConsole(pConsole) {}
		~Debug() = default;

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
		void Log(const std::string& message, const LogLevel& logLevel, bool bIncludeTimeStamp = true);
		void LogInfo(const std::string& message, bool bIncludeTimeStamp = true);
		void LogNotice(const std::string& message, bool bIncludeTimeStamp = true);
		void LogWarning(const std::string& message, bool bIncludeTimeStamp = true);
		void LogError(const std::string& message, bool bIncludeTimeStamp = true);
		void LogFatalError(const std::string& message, bool bIncludeTimeStamp = true);

		void LogOnce(const std::string& key, const std::string& message, const LogLevel& logLevel, bool bIncludeTimeStamp = true);

#ifndef GLORY_NO_DEBUG_LINES

		void DrawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color, float time = 0.1f);
		void DrawLineQuad(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec3& p4, const glm::vec4& color, float time = 0.1f);
		void DrawWireCube(const glm::vec3& position, const glm::vec3& extends, const glm::vec4& color, float time = 0.1f);
		void DrawRay(const glm::vec3& start, const glm::vec3& dir, const glm::vec4& color, float length = 1.0f, float time = 0.1f);

#endif

	private:
		friend class Engine;
		friend class RendererModule;

		std::mutex m_Lock;
		Console* m_pConsole;

#ifndef GLORY_NO_DEBUG_LINES

		void SubmitLines(RendererModule* pRenderer, GameTime* pTime);

#endif

#ifndef GLORY_NO_DEBUG_LINES

		struct DebugLine
		{
			glm::vec3 Start;
			glm::vec3 End;
			glm::vec4 Color;
			float Time;
		};
		std::vector<DebugLine> m_DebugLines;

#endif
	};
}