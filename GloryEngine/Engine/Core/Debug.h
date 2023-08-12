#pragma once
#include <string>
#include <vector>

#ifndef GLORY_NO_DEBUG_LINES

#include <glm/glm.hpp>

#endif

#include "Glory.h"

namespace Glory
{
	class RendererModule;

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

		static void LogOnce(const std::string& key, const std::string& message, const LogLevel& logLevel, bool bIncludeTimeStamp = true);

#ifndef GLORY_NO_DEBUG_LINES

		static void DrawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color, float time = 0.1f);
		static void DrawLineQuad(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec3& p4, const glm::vec4& color, float time = 0.1f);
		static void DrawWireCube(const glm::vec3& position, const glm::vec3& extends, const glm::vec4& color, float time = 0.1f);
		static void DrawRay(const glm::vec3& start, const glm::vec3& dir, const glm::vec4& color, float length = 1.0f, float time = 0.1f);

#endif

	private:
		friend class GloryContext;
		friend class RendererModule;
		Debug() = default;
		~Debug() = default;

#ifndef GLORY_NO_DEBUG_LINES

		static void SubmitLines(RendererModule* pRenderer);

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