#include "Debug.h"
#include "Console.h"
#include "GameTime.h"
#include "WindowModule.h"
#include "RendererModule.h"

#include <ThreadedVar.h>

#ifndef GLORY_NO_DEBUG_LINES

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#endif

#define DEBUG GloryContext::GetDebug()

namespace Glory
{
	ThreadedVector<std::string> m_LoggedKeys;

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
		std::unique_lock lock{m_Lock};
		m_pConsole->SetNextColor({1.f, 1.f, 1.f, 1.0f});
		std::string finalMessage = "Info:	" + message;
		m_pConsole->WriteLine(finalMessage, bIncludeTimeStamp);
	}

	void Debug::LogNotice(const std::string& message, bool bIncludeTimeStamp)
	{
		std::unique_lock lock{m_Lock};
		m_pConsole->SetNextColor({ 0.5f, 0.5f, 0.5f, 1.0f });
		std::string finalMessage = "Notice:	" + message;
		m_pConsole->WriteLine(finalMessage, bIncludeTimeStamp);
	}

	void Debug::LogWarning(const std::string& message, bool bIncludeTimeStamp)
	{
		std::unique_lock lock{m_Lock};
		m_pConsole->SetNextColor({ 1.f, 0.918f, 0.0f, 1.0f });
		std::string finalMessage = "WARNING:	" + message;
		m_pConsole->WriteLine(finalMessage, bIncludeTimeStamp);
	}

	void Debug::LogError(const std::string& message, bool bIncludeTimeStamp)
	{
		std::unique_lock lock{m_Lock};
		m_pConsole->SetNextColor({ 1.f, 0.0f, 0.0f, 1.0f });
		std::string finalMessage = "ERROR:	" + message;
		m_pConsole->WriteLine(finalMessage, bIncludeTimeStamp);
	}

	void Debug::LogFatalError(const std::string& message, bool bIncludeTimeStamp)
	{
		std::unique_lock lock{m_Lock};
		m_pConsole->SetNextColor({ 1.f, 0.0f, 0.0f, 1.0f });
		std::string finalMessage = "FATAL ERROR:	" + message;
		m_pConsole->WriteLine(finalMessage, bIncludeTimeStamp);

		// Open message box
		if (m_pWindowModule) m_pWindowModule->OpenMessageBox("FATAL ERROR: " + message);
		exit(-1);
		//Game::GetGame().Quit();
	}

	void Debug::LogOnce(const std::string& key, const std::string& message, const LogLevel& logLevel, bool bIncludeTimeStamp)
	{
		if (m_LoggedKeys.Contains(key)) return;
		m_LoggedKeys.push_back(key);
		Log(message, logLevel, bIncludeTimeStamp);
	}

	void Debug::SetWindowModule(WindowModule* pWindows)
	{
		m_pWindowModule = pWindows;
	}

#ifndef GLORY_NO_DEBUG_LINES

	void Debug::DrawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color, float time)
	{
		m_DebugLines.push_back({ start , end, color, time });
	}

	void Debug::DrawLineQuad(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec3& p4, const glm::vec4& color, float time)
	{
		DrawLine(p1, p2, color, time);
		DrawLine(p2, p3, color, time);
		DrawLine(p3, p4, color, time);
		DrawLine(p4, p1, color, time);
	}

	void Debug::DrawWireCube(const glm::vec3& position, const glm::vec3& extends, const glm::vec4& color, float time)
	{
		const glm::vec3 topTopLeft = position + glm::vec3(-extends.x, extends.y, -extends.z);
		const glm::vec3 topTopRight = position + glm::vec3(extends.x, extends.y, -extends.z);
		const glm::vec3 topBottomRight = position + glm::vec3(extends.x, extends.y, extends.z);
		const glm::vec3 topBottomLeft = position + glm::vec3(-extends.x, extends.y, extends.z);

		const glm::vec3 bottomTopLeft = position + glm::vec3(-extends.x, -extends.y, -extends.z);
		const glm::vec3 bottomTopRight = position + glm::vec3(extends.x, -extends.y, -extends.z);
		const glm::vec3 bottomBottomRight = position + glm::vec3(extends.x, -extends.y, extends.z);
		const glm::vec3 bottomBottomLeft = position + glm::vec3(-extends.x, -extends.y, extends.z);

		DrawLineQuad(topTopLeft, topTopRight, topBottomRight, topBottomLeft, color, time);
		DrawLineQuad(bottomTopLeft, bottomTopRight, bottomBottomRight, bottomBottomLeft, color, time);

		DrawLineQuad(topBottomLeft, topTopLeft, bottomTopLeft, bottomBottomLeft, color, time);
		DrawLineQuad(topBottomRight, topTopRight, bottomTopRight, bottomBottomRight, color, time);
	}

	void Debug::DrawRay(const glm::vec3& start, const glm::vec3& dir, const glm::vec4& color, float length, float time)
	{
		DrawLine(start, start + dir * length, color, time);
	}


	void Debug::SubmitLines(RendererModule* pRenderer, GameTime* pTime)
	{
		for (size_t i = m_DebugLines.size(); i > 0; --i)
		{
			const size_t index = i - 1;
			DebugLine& line = m_DebugLines[index];
			line.Time -= pTime->GetUnscaledDeltaTime();
			if (line.Time <= 0.f)
			{
				m_DebugLines.erase(m_DebugLines.begin() + index);
				continue;
			}

			pRenderer->DrawLine(glm::identity<glm::mat4>(), line.Start, line.End, line.Color);
		}
	}

#endif
}