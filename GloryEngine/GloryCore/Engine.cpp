#include "Engine.h"

namespace Glory
{
	Engine* Engine::CreateEngine(const EngineCreateInfo& createInfo)
	{
		Engine* pEngine = new Engine(createInfo);
		return pEngine;
	}

	Engine::Engine(const EngineCreateInfo& createInfo) : m_pWindowModule(createInfo.pWindowModule)
	{
		// Copy the optional modules into the optional modules vector
		if (createInfo.OptionalModuleCount > 0 && createInfo.pOptionalModules != nullptr)
		{
			m_pOptionalModules.resize(createInfo.OptionalModuleCount);
			memcpy(m_pOptionalModules.data(), createInfo.pOptionalModules, (size_t)createInfo.OptionalModuleCount);
		}

		// Fill in the all modules vector with the required modules first
		// In order of importance
		m_pAllModules.push_back(m_pWindowModule);

		// Add optional modules
		size_t currentSize = m_pAllModules.size();
		m_pAllModules.resize(currentSize + m_pOptionalModules.size());
		memcpy(&m_pAllModules[0], m_pOptionalModules.data(), m_pOptionalModules.size());
	}

	Engine::~Engine()
	{
		// We need to cleanup in reverse
		// This makes sure things like graphics get cleaned up before we close the window
		for (int i = (int)m_pAllModules.size() - 1; i >= 0; --i)
		{
			m_pAllModules[(size_t)i]->Cleanup();
			delete m_pAllModules[(size_t)i];
		}
		m_pWindowModule = nullptr;

		m_pAllModules.clear();
		m_pOptionalModules.clear();
	}

	void Engine::Initialize()
	{
		for (size_t i = 0; i < m_pAllModules.size(); i++)
		{
			m_pAllModules[i]->Initialize();
		}
	}

	void Engine::Update()
	{
		for (size_t i = 0; i < m_pAllModules.size(); i++)
		{
			m_pAllModules[i]->Update();
		}
	}

	void Engine::Draw()
	{
		for (size_t i = 0; i < m_pAllModules.size(); i++)
		{
			m_pAllModules[i]->Draw();
		}
	}
}
