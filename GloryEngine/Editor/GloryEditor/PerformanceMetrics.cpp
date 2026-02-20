#include "PerformanceMetrics.h"
#include "EditorApplication.h"

#include <GameTime.h>
#include <imgui.h>
#include <Engine.h>
#include <GraphicsDevice.h>
#include <Renderer.h>

namespace Glory::Editor
{
	const float PerformanceMetrics::REFRESHINTERVALS = 0.1f;

	PerformanceMetrics::PerformanceMetrics() : EditorWindowTemplate("Performance Metrics", 200.0f, 200.0f), 
		m_LastFrameTime(0.0f), m_LastFramerate(0.0f) {}

	PerformanceMetrics::~PerformanceMetrics() {}

	void PerformanceMetrics::OnGUI()
	{
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();

		std::chrono::time_point<std::chrono::system_clock> currentTime = std::chrono::system_clock::now();
		std::chrono::duration<float> frameDuration = currentTime - m_LastRefresh;
		float timeSinceRefresh = std::chrono::duration_cast<std::chrono::seconds>(frameDuration).count();
		if (timeSinceRefresh >= REFRESHINTERVALS)
			Refresh();

		ImGui::Text("Average Frame Time: %.3f ms - Framerate: %.1f", m_LastFrameTime, m_LastFramerate);
		ImGui::Text("Frame Count: %d", pEngine->Time().GetTotalFrames());

		GraphicsDevice* pDevice = pEngine->ActiveGraphicsDevice();
		ImGui::Text("Draw Calls: %d - Vertices: %d - Triangles: %d", pDevice->GetLastDrawCalls(), pDevice->GetLastVertexCount(), pDevice->GetLastTriangleCount());

		Renderer* pRenderer = pEngine->ActiveRenderer();
		ImGui::Text("Active Objects: %d - Active Cameras: %d", pRenderer->LastSubmittedObjectCount(), pRenderer->LastSubmittedCameraCount());
	}

	void PerformanceMetrics::Refresh()
	{
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();

		m_LastFrameTime = pEngine->Time().GetUnscaledDeltaTime();
		m_LastFramerate = pEngine->Time().GetFrameRate();

		m_LastRefresh = std::chrono::system_clock::now();
	}
}
