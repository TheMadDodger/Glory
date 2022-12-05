#include "PerformanceMetrics.h"
#include <GameTime.h>
#include <imgui.h>
#include <TimerModule.h>
#include <Game.h>
#include <Engine.h>
#include <GraphicsModule.h>

namespace Glory::Editor
{
	const float PerformanceMetrics::REFRESHINTERVALS = 0.1f;

	PerformanceMetrics::PerformanceMetrics() : EditorWindowTemplate("Performance Metrics", 200.0f, 200.0f), 
		m_LastFrameTime(0.0f), m_LastFramerate(0.0f), m_LastGameThreadFrameTime(0.0f), m_LastGraphicsThreadFrameTime(0.0f) {}

	PerformanceMetrics::~PerformanceMetrics() {}

	void PerformanceMetrics::OnGUI()
	{
		std::chrono::time_point<std::chrono::system_clock> currentTime = std::chrono::system_clock::now();
		std::chrono::duration<float> frameDuration = currentTime - m_LastRefresh;
		float timeSinceRefresh = std::chrono::duration_cast<seconds>(frameDuration).count();
		if (timeSinceRefresh >= REFRESHINTERVALS)
			Refresh();

		ImGui::Text("Average Frame Time: %.3f ms - Framerate: %.1f", m_LastFrameTime, m_LastFramerate);
		ImGui::Text("Average Game Time: %.3f ms - Average Graphics Time: %.3f ms", m_LastGameThreadFrameTime, m_LastGraphicsThreadFrameTime);
		ImGui::Text("Frame Count: %d - Game Frame Count: %d", Time::GetTotalFrames(), Time::GetTotalGameFrames());

		GraphicsModule* pGraphics = Game::GetGame().GetEngine()->GetGraphicsModule();
		ImGui::Text("Draw Calls: %d - Vertices: %d - Triangles: %d", pGraphics->GetLastDrawCalls(), pGraphics->GetLastVertexCount(), pGraphics->GetLastTriangleCount());

		RendererModule* pRenderer = Game::GetGame().GetEngine()->GetRendererModule();
		ImGui::Text("Active Objects: %d - Active Cameras: %d", pRenderer->LastSubmittedObjectCount(), pRenderer->LastSubmittedCameraCount());
	}

	void PerformanceMetrics::Refresh()
	{
		m_LastFrameTime = Time::GetUnscaledDeltaTime<float, std::milli>();
		m_LastFramerate = Time::GetFrameRate();
		m_LastGameThreadFrameTime = Time::GetUnscaledGameDeltaTime<float, std::milli>();
		m_LastGraphicsThreadFrameTime = Time::GetUnscaledGraphicsDeltaTime<float, std::milli>();

		m_LastRefresh = std::chrono::system_clock::now();
	}
}
