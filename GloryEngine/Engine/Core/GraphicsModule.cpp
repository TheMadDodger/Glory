#include "GraphicsModule.h"
#include "GraphicsThread.h"
#include "FrameStates.h"
#include "Engine.h"

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

	void GraphicsModule::DrawMesh(MeshData* pMeshData, uint32_t vertexOffset, uint32_t vertexCount)
	{
		m_CurrentVertices += vertexCount;
		m_CurrentTriangles += vertexCount / 3;
		++m_CurrentDrawCalls;
		Mesh* pMesh = GetResourceManager()->CreateMesh(pMeshData);
		OnDrawMesh(pMesh, vertexOffset, vertexCount);
	}

	void GraphicsModule::DrawMesh(Mesh* pMesh, uint32_t vertexOffset, uint32_t vertexCount)
	{
		m_CurrentVertices += vertexCount;
		if (pMesh->GetPrimitiveType() == PrimitiveType::PT_Triangles)
			m_CurrentTriangles += vertexCount / 3;
		++m_CurrentDrawCalls;
		OnDrawMesh(pMesh, vertexOffset, vertexCount);
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

	void GraphicsModule::PostInitialize()
	{
		/* Bind to graphics thread */
		m_pEngine->GetGraphicsThread()->BindNoRender(this);
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