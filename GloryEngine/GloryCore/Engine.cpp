#include "Engine.h"
#include "Console.h"
#include <algorithm>

namespace Glory
{
	Engine* Engine::CreateEngine(const EngineCreateInfo& createInfo)
	{
		Engine* pEngine = new Engine(createInfo);
		return pEngine;
	}

	WindowModule* Engine::GetWindowModule() const
	{
		return m_pWindowModule;
	}

	ScenesModule* Engine::GetScenesModule() const
	{
		return m_pScenesModule;
	}

	RendererModule* Engine::GetRendererModule() const
	{
		return m_pRenderModule;
	}

	GraphicsModule* Engine::GetGraphicsModule() const
	{
		return m_pGraphicsModule;
	}

	Module* Engine::GetModule(const std::type_info& type)
	{
		auto it = std::find_if(m_pAllModules.begin(), m_pAllModules.end(), [&](Module* pModule)
		{
			return pModule->GetModuleType() == type;
		});

		if (it == m_pAllModules.end()) return nullptr;
		return *it;
	}

	GameThread* Engine::GetGameThread() const
	{
		return m_pGameThread;
	}

	GraphicsThread* Engine::GetGraphicsThread() const
	{
		return m_pGraphicsThread;
	}

	Engine::Engine(const EngineCreateInfo& createInfo)
		: m_pWindowModule(createInfo.pWindowModule), m_pGraphicsModule(createInfo.pGraphicsModule),
		m_pThreadManager(ThreadManager::GetInstance()), m_pJobManager(Jobs::JobManager::GetInstance()),
		m_pScenesModule(createInfo.pScenesModule), m_pRenderModule(createInfo.pRenderModule),
		m_pGameThread(nullptr), m_pGraphicsThread(nullptr)
	{
		// Copy the optional modules into the optional modules vector
		if (createInfo.OptionalModuleCount > 0 && createInfo.pOptionalModules != nullptr)
		{
			m_pOptionalModules.resize(createInfo.OptionalModuleCount);
			for (size_t i = 0; i < createInfo.OptionalModuleCount; i++)
			{
				m_pOptionalModules[i] = createInfo.pOptionalModules[i];
				if (m_pOptionalModules[i]->HasPriority())
					m_pPriorityInitializationModules.push_back(m_pOptionalModules[i]);
			}
		}

		// Fill in the all modules vector with the required modules first
		// In order of importance
		m_pAllModules.push_back(m_pWindowModule);
		m_pAllModules.push_back(m_pScenesModule);
		m_pAllModules.push_back(m_pRenderModule);
		m_pAllModules.push_back(m_pGraphicsModule);

		// Add optional modules
		size_t currentSize = m_pAllModules.size();
		m_pAllModules.resize(currentSize + m_pOptionalModules.size());
		for (size_t i = 0; i < m_pOptionalModules.size(); i++)
		{
			m_pAllModules[currentSize + i] = m_pOptionalModules[i];
		}
	}

	Engine::~Engine()
	{
		Console::Cleanup();

		m_pGameThread->Stop();
		m_pGraphicsThread->Stop();

		m_pJobManager->Kill();
		delete m_pJobManager;
		m_pJobManager = nullptr;

		m_pThreadManager->Destroy();

		// We need to cleanup in reverse
		// This makes sure things like graphics get cleaned up before we close the window
		for (int i = (int)m_pAllModules.size() - 1; i >= 0; --i)
		{
			m_pAllModules[(size_t)i]->Cleanup();
			delete m_pAllModules[(size_t)i];
		}
		m_pWindowModule = nullptr;
		m_pScenesModule = nullptr;
		m_pRenderModule = nullptr;
		m_pGraphicsModule = nullptr;

		m_pAllModules.clear();
		m_pOptionalModules.clear();
		m_pPriorityInitializationModules.clear();

		delete m_pMainThread;
		delete m_pGameThread;
		delete m_pGraphicsThread;
	}

	void Engine::Initialize()
	{
		Console::Initialize();

		for (size_t i = 0; i < m_pPriorityInitializationModules.size(); i++)
		{
			m_pPriorityInitializationModules[i]->m_pEngine = this;
			m_pPriorityInitializationModules[i]->Initialize();
		}
		
		for (size_t i = 0; i < m_pAllModules.size(); i++)
		{
			auto it = std::find(m_pPriorityInitializationModules.begin(), m_pPriorityInitializationModules.end(), m_pAllModules[i]);
			if (it != m_pPriorityInitializationModules.end()) continue;
			m_pAllModules[i]->m_pEngine = this;
			m_pAllModules[i]->Initialize();
		}

		m_pMainThread = new MainThread();
		m_pGameThread = new GameThread(this);
		m_pGraphicsThread = new GraphicsThread();

		m_pMainThread->Bind<WindowModule>(m_pWindowModule);

		m_pGameThread->Bind<ScenesModule>(m_pScenesModule);
		m_pGraphicsThread->Bind<RendererModule>(m_pRenderModule);
		m_pGraphicsThread->BindNoRender<GraphicsModule>(m_pGraphicsModule);

		m_pGraphicsThread->Start();
		m_pGameThread->Start();
	}

	void Engine::Update()
	{
		Console::Update();
		m_pMainThread->Update();
	}
}
