#include "Engine.h"
#include "Console.h"
#include "AssetManager.h"
#include "Serializer.h"
#include "PropertySerializer.h"
#include "AssetReferencePropertySerializer.h"
#include "ArrayPropertySerializer.h"
#include "EnumPropertySerializer.h"
#include "StructPropertySerializer.h"
#include "SceneObjectRefSerializer.h"
#include "SerializedPropertyManager.h"
#include "ShaderManager.h"
#include "ScriptExtensions.h"
#include "ScriptingExtender.h"
#include "GloryContext.h"
#include "WindowsDebugConsole.h"
#include "LayerRef.h"
#include "SceneObjectRef.h"
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

	InputModule* Engine::GetInputModule() const
	{
		return m_pInputModule;
	}

	TimerModule* Engine::GetTimerModule() const
	{
		return m_pTimerModule;
	}

	ProfilerModule* Engine::GetProfilerModule() const
	{
		return m_pProfilerModule;
	}

	ScriptingExtender* Engine::GetScriptingExtender() const
	{
		return m_pScriptingExtender;
	}

	LoaderModule* Engine::GetLoaderModule(const std::string& extension)
	{
		const ResourceType* pResourceType = ResourceType::GetResourceType(extension);
		if (!pResourceType) return nullptr;
		return GetLoaderModule(pResourceType->Hash());
	}

	LoaderModule* Engine::GetLoaderModule(const std::type_info& resourceType)
	{
		if (m_TypeToLoader.find(resourceType) == m_TypeToLoader.end()) return nullptr;
		size_t loaderIndex = m_TypeToLoader[resourceType];
		return m_pLoaderModules[loaderIndex];
	}

	LoaderModule* Engine::GetLoaderModule(uint32_t typeHash)
	{
		if (m_TypeHashToLoader.find(typeHash) == m_TypeHashToLoader.end()) return nullptr;
		size_t loaderIndex = m_TypeHashToLoader[typeHash];
		return m_pLoaderModules[loaderIndex];
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

	Module* Engine::GetModule(const std::string& name)
	{
		auto it = std::find_if(m_pAllModules.begin(), m_pAllModules.end(), [&](Module* pModule)
		{
			return pModule->GetMetaData().Name() == name;
		});

		if (it == m_pAllModules.end()) return nullptr;
		return *it;
	}

	GraphicsThread* Engine::GetGraphicsThread() const
	{
		return m_pGraphicsThread;
	}

	void Engine::StartThreads()
	{
		m_pGraphicsThread->Start();
	}

	Engine::Engine(const EngineCreateInfo& createInfo)
		: m_pWindowModule(createInfo.pWindowModule), m_pGraphicsModule(createInfo.pGraphicsModule),
		m_pThreadManager(ThreadManager::GetInstance()), m_pJobManager(Jobs::JobManager::GetInstance()),
		m_pScenesModule(createInfo.pScenesModule), m_pRenderModule(createInfo.pRenderModule),
		m_pTimerModule(new TimerModule()), m_pProfilerModule(new ProfilerModule()), m_pInputModule(createInfo.pInputModule),
		m_pGraphicsThread(nullptr), m_pScriptingExtender(new ScriptingExtender()), m_CreateInfo(createInfo)
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
		if (m_pWindowModule) m_pAllModules.push_back(m_pWindowModule);
		if (m_pScenesModule) m_pAllModules.push_back(m_pScenesModule);
		if (m_pRenderModule) m_pAllModules.push_back(m_pRenderModule);
		if (m_pGraphicsModule) m_pAllModules.push_back(m_pGraphicsModule);
		if (m_pInputModule) m_pAllModules.push_back(m_pInputModule);
		if (m_pTimerModule) m_pAllModules.push_back(m_pTimerModule);

		// Add optional modules
		size_t currentSize = m_pAllModules.size();
		m_pAllModules.resize(currentSize + m_pOptionalModules.size());
		for (size_t i = 0; i < m_pOptionalModules.size(); i++)
		{
			m_pAllModules[currentSize + i] = m_pOptionalModules[i];
		}

		m_pScriptingModules.resize(createInfo.ScriptingModulesCount);
		if (m_pScriptingModules.size() > 0) memcpy(&m_pScriptingModules[0], createInfo.pScriptingModules, createInfo.ScriptingModulesCount * sizeof(ScriptingModule*));

		currentSize = m_pAllModules.size();
		m_pAllModules.resize(currentSize + m_pScriptingModules.size() * 2);
		for (size_t i = 0; i < m_pScriptingModules.size(); ++i)
		{
			m_pAllModules[currentSize + i] = m_pScriptingModules[i];
			m_pAllModules[currentSize + m_pScriptingModules.size() + i] = m_pScriptingModules[i]->CreateLoaderModule();
		}

		m_pAllModules.push_back(m_pProfilerModule);
	}

	Engine::~Engine()
	{
		AssetManager::Destroy();

		m_pGraphicsThread->Stop();

		m_pJobManager->Kill();

		m_pThreadManager->Destroy();

		delete m_pJobManager;
		m_pJobManager = nullptr;

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
		m_TypeToLoader.clear();
		m_TypeHashToLoader.clear();
		m_pLoaderModules.clear();

		delete m_pGraphicsThread;
		m_pGraphicsThread = nullptr;

		delete m_pScriptingExtender;
		m_pScriptingExtender = nullptr;

		Serializer::Cleanup();
		PropertySerializer::Cleanup();
		SerializedPropertyManager::Clear();
		ShaderManager::Cleanup();
		GloryContext::DestroyContext();
	}

	void Engine::Initialize()
	{
		GloryContext::GetContext()->Initialize();

		RegisterBasicTypes();
		RegisterStandardSerializers();

		for (size_t i = 0; i < m_pPriorityInitializationModules.size(); i++)
		{
			m_pPriorityInitializationModules[i]->m_pEngine = this;
			m_pPriorityInitializationModules[i]->Initialize();
		}

		for (size_t i = 0; i < m_pAllModules.size(); i++)
		{
			LoaderModule* pLoaderModule = dynamic_cast<LoaderModule*>(m_pAllModules[i]);
			if (pLoaderModule)
			{
				size_t index = m_pLoaderModules.size();
				m_pLoaderModules.push_back(pLoaderModule);
				std::type_index type = pLoaderModule->GetResourceType();
				m_TypeToLoader[type] = index;
				uint32_t typeHash = ResourceType::GetHash(type);
				m_TypeHashToLoader[typeHash] = index;
			}

			for (size_t j = 0; j < m_pAllModules[i]->m_pScriptingExtender.size(); j++)
			{
				IScriptExtender* pScriptExtender = m_pAllModules[i]->m_pScriptingExtender[j];
				m_pScriptingExtender->RegisterManagedExtender(m_pAllModules[i], pScriptExtender);
			}

			auto it = std::find(m_pPriorityInitializationModules.begin(), m_pPriorityInitializationModules.end(), m_pAllModules[i]);
			if (it != m_pPriorityInitializationModules.end()) continue;
			m_pAllModules[i]->m_pEngine = this;
			m_pAllModules[i]->Initialize();
		}

		AssetManager::Initialize();

		m_pScriptingExtender->Initialize(this);

		// Run Post Initialize
		for (size_t i = 0; i < m_pAllModules.size(); i++)
		{
			m_pAllModules[i]->PostInitialize();
		}

		// Bind rendering to Graphics Thread
		m_pGraphicsThread = new GraphicsThread(this);
		if(m_pGraphicsModule) m_pGraphicsThread->BindNoRender<GraphicsModule>(m_pGraphicsModule);
		if(m_pRenderModule) m_pGraphicsThread->Bind<RendererModule>(m_pRenderModule);
	}

	void Engine::RegisterStandardSerializers()
	{
		// Standard
		STANDARD_SERIALIZER(int);
		STANDARD_SERIALIZER(float);
		STANDARD_SERIALIZER(double);
		STANDARD_SERIALIZER(bool);
		STANDARD_SERIALIZER(long);
		STANDARD_SERIALIZER(uint32_t);
		STANDARD_SERIALIZER(uint64_t);
		STANDARD_SERIALIZER(glm::vec3);
		STANDARD_SERIALIZER(glm::vec4);
		STANDARD_SERIALIZER(glm::quat);
		STANDARD_SERIALIZER(LayerMask);
		STANDARD_SERIALIZER(LayerRef);
		STANDARD_SERIALIZER(SceneObjectRef);

		// Special
		PropertySerializer::RegisterSerializer<AssetReferencePropertySerializer>();
		PropertySerializer::RegisterSerializer<ArrayPropertySerializer>();
		PropertySerializer::RegisterSerializer<EnumPropertySerializer>();
		PropertySerializer::RegisterSerializer<StructPropertySerializer>();
		PropertySerializer::RegisterSerializer<SceneObjectRefSerializer>();
	}

	void Engine::RegisterBasicTypes()
	{
		ResourceType::RegisterType<int>();
		ResourceType::RegisterType<float>();
		ResourceType::RegisterType<double>();
		ResourceType::RegisterType<long>();
		ResourceType::RegisterType<glm::vec2>();
		ResourceType::RegisterType<glm::vec3>();
		ResourceType::RegisterType<glm::vec4>();
		ResourceType::RegisterType<glm::quat>();
		ResourceType::RegisterType<LayerMask>();
		ResourceType::RegisterType<SceneObjectRef>();

		GloryReflect::Reflect::RegisterBasicType<UUID>("UUID");
		GloryReflect::Reflect::RegisterBasicType<LayerRef>("LayerRef");
		GloryReflect::Reflect::RegisterBasicType<LayerMask>();
		GloryReflect::Reflect::RegisterEnum<InputDeviceType>();
		GloryReflect::Reflect::RegisterEnum<InputMappingType>();
		GloryReflect::Reflect::RegisterEnum<InputState>();
		GloryReflect::Reflect::RegisterEnum<AxisBlending>();

		GloryReflect::Reflect::RegisterTemplatedType("AssetReference,Glory::AssetReference,class Glory::AssetReference", ST_Asset, sizeof(UUID));
	}

	void Engine::Update()
	{
		GameThreadFrameStart();
		Console::Update();
		m_pWindowModule->PollEvents();
		ModulesLoop();
		GameThreadFrameEnd();
	}

	void Engine::ModulesLoop(IModuleLoopHandler* pLoopHandler)
	{
		AssetManager::RunCallbacks();

		for (size_t i = 0; i < m_pAllModules.size(); i++)
		{
			Module* pModule = m_pAllModules[i];

			if (pLoopHandler != nullptr && pLoopHandler->HandleModuleLoop(pModule)) continue;

			m_pAllModules[i]->Update();
			m_pAllModules[i]->Draw();
		}
	}

	void Engine::GameThreadFrameStart()
	{
		for (size_t i = 0; i < m_pAllModules.size(); i++)
		{
			m_pAllModules[i]->OnGameThreadFrameStart();
		}
	}

	void Engine::GameThreadFrameEnd()
	{
		for (size_t i = 0; i < m_pAllModules.size(); i++)
		{
			m_pAllModules[i]->OnGameThreadFrameEnd();
		}
	}

	void Engine::CallModuleUpdate(Module* pModule)
	{
		pModule->Update();
	}

	void Engine::CallModuleDraw(Module* pModule)
	{
		pModule->Draw();
	}

	const size_t Engine::ModulesCount() const
	{
		return m_pAllModules.size();
	}

	Module* Engine::GetModule(size_t index) const
	{
		if (index >= m_pAllModules.size()) return nullptr;
		return m_pAllModules[index];
	}

	void Engine::LoadModuleSettings(const std::filesystem::path& overrideRootPath)
	{
		for (size_t i = 0; i < m_pAllModules.size(); i++)
		{
			Module* pModule = m_pAllModules[i];
			const ModuleMetaData& moduleMetaData = pModule->GetMetaData();

			const std::filesystem::path modulePath = moduleMetaData.Path();
			/* Ignore built-in modules */
			if (modulePath.empty()) continue;

			if (overrideRootPath.empty())
			{
				std::filesystem::path settingsFilePath = modulePath;
				settingsFilePath.append("config.yaml");
				pModule->LoadSettings(settingsFilePath);
				continue;
			}

			std::filesystem::path settingsFilePath = overrideRootPath;
			settingsFilePath.append(moduleMetaData.Name() + ".yaml");
			pModule->LoadSettings(settingsFilePath);
		}
	}

	void Engine::GraphicsThreadFrameStart()
	{
		for (size_t i = 0; i < m_pAllModules.size(); i++)
		{
			m_pAllModules[i]->OnGraphicsThreadFrameStart();
		}
	}

	void Engine::GraphicsThreadFrameEnd()
	{
		for (size_t i = 0; i < m_pAllModules.size(); i++)
		{
			m_pAllModules[i]->OnGraphicsThreadFrameEnd();
		}
	}
}
