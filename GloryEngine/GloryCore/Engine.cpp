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

	WindowModule* Engine::GetWindowModule()
	{
		return m_pWindowModule;
	}

	GraphicsModule* Engine::GetGraphicsModule()
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

	Engine::Engine(const EngineCreateInfo& createInfo) : m_pWindowModule(createInfo.pWindowModule), m_pGraphicsModule(createInfo.pGraphicsModule)
	{
		// Copy the optional modules into the optional modules vector
		if (createInfo.OptionalModuleCount > 0 && createInfo.pOptionalModules != nullptr)
		{
			m_pOptionalModules.resize(createInfo.OptionalModuleCount);
			for (size_t i = 0; i < createInfo.OptionalModuleCount; i++)
			{
				m_pOptionalModules[i] = createInfo.pOptionalModules[i];
			}
		}

		// Fill in the all modules vector with the required modules first
		// In order of importance
		m_pAllModules.push_back(m_pWindowModule);
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
		Console::Initialize();

		for (size_t i = 0; i < m_pAllModules.size(); i++)
		{
			m_pAllModules[i]->Initialize();
		}
	}

	void Engine::Update()
	{
		Console::Update();

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
