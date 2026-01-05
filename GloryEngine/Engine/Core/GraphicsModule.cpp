#include "GraphicsModule.h"
#include "Engine.h"

namespace Glory
{
	GraphicsModule::GraphicsModule() : m_CurrentDrawCalls(0), m_LastDrawCalls(0),
		m_LastVertices(0), m_CurrentVertices(0), m_LastTriangles(0), m_CurrentTriangles(0) {}

	GraphicsModule::~GraphicsModule()
	{
	}

	const std::type_info& GraphicsModule::GetBaseModuleType()
	{
		return typeid(GraphicsModule);
	}

	int GraphicsModule::GetLastDrawCalls()
	{
		return m_LastDrawCalls;
	}

	int GraphicsModule::GetLastVertexCount()
	{
		return m_LastVertices;
	}

	int GraphicsModule::GetLastTriangleCount()
	{
		return m_LastTriangles;
	}

	void GraphicsModule::OnBeginFrame()
	{
		m_CurrentDrawCalls = 0;
		m_CurrentVertices = 0;
		m_CurrentTriangles = 0;
	}

	void GraphicsModule::OnEndFrame()
	{
		m_LastDrawCalls = m_CurrentDrawCalls;
		m_LastVertices = m_CurrentVertices;
		m_LastTriangles = m_CurrentTriangles;
	}
}