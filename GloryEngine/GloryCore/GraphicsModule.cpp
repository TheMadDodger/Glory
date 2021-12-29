#include "GraphicsModule.h"
#include "FrameStates.h"

namespace Glory
{
	GraphicsModule::GraphicsModule() : m_pFrameStates(nullptr), m_pResourceManager(nullptr), m_CurrentDrawCalls(0), m_LastDrawCalls(0),
		m_LastVertices(0), m_CurrentVertices(0), m_LastTriangles(0), m_CurrentTriangles(0) {}

	GraphicsModule::~GraphicsModule()
	{
		delete m_pFrameStates;
		m_pFrameStates = nullptr;
	}

	const std::type_info& GraphicsModule::GetModuleType()
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

	void GraphicsModule::DrawMesh(MeshData* pMeshData)
	{
		int vertexCount = pMeshData->VertexCount();
		m_CurrentVertices += vertexCount;
		m_CurrentTriangles += vertexCount / 3;
		++m_CurrentDrawCalls;
		OnDrawMesh(pMeshData);
	}

	FrameStates* GraphicsModule::GetFrameStates()
	{
		return m_pFrameStates;
	}

	GPUResourceManager* GraphicsModule::GetResourceManager()
	{
		return m_pResourceManager;
	}

	void GraphicsModule::ThreadedCleanup()
	{
		delete m_pResourceManager;
		m_pResourceManager = nullptr;
	}

	FrameStates* GraphicsModule::CreateFrameStates()
	{
		return new FrameStates(this);
	}

	void GraphicsModule::Initialize()
	{
		m_pFrameStates = CreateFrameStates();
		m_pResourceManager = CreateGPUResourceManager();
		OnInitialize();
		m_pFrameStates->Initialize();
	}

	void GraphicsModule::Cleanup()
	{
		OnCleanup();
	}

	void GraphicsModule::OnGraphicsThreadFrameStart()
	{
		m_CurrentDrawCalls = 0;
		m_CurrentVertices = 0;
		m_CurrentTriangles = 0;
	}

	void GraphicsModule::OnGraphicsThreadFrameEnd()
	{
		m_LastDrawCalls = m_CurrentDrawCalls;
		m_LastVertices = m_CurrentVertices;
		m_LastTriangles = m_CurrentTriangles;
	}
}