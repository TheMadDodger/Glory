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
#include "ShapePropertySerializer.h"
#include "ShaderManager.h"
#include "ScriptExtensions.h"
#include "ScriptingExtender.h"
#include "GloryContext.h"
#include "LayerRef.h"
#include "SceneObjectRef.h"
#include "Physics.h"
#include "ShapeProperty.h"
#include "PrefabData.h"
#include "SceneManager.h"

#include "IModuleLoopHandler.h"
#include "GraphicsThread.h"
#include "ResourceLoaderModule.h"

#include "TimerModule.h"
#include "ProfilerModule.h"

#include "ScriptingModule.h"
#include "IScriptExtender.h"

#include <JobManager.h>
#include <ThreadManager.h>

#include <algorithm>

namespace Glory
{
	Engine* Engine::CreateEngine(const EngineCreateInfo& createInfo)
	{
		Engine* pEngine = new Engine(createInfo);
		return pEngine;
	}

	SceneManager* Engine::GetSceneManager()
	{
		return m_pSceneManager;
	}

	void Engine::AddMainModule(Module* pModule, bool initialize)
	{
		m_pMainModules.push_back(pModule);
		m_pAllModules.push_back(pModule);
		if (!initialize) return;
		pModule->Initialize();
		pModule->PostInitialize();
		pModule->m_IsInitialized = true;
	}

	void Engine::AddOptionalModule(Module* pModule, bool initialize)
	{
		m_pOptionalModules.push_back(pModule);
		m_pAllModules.push_back(pModule);
		if (!initialize) return;
		pModule->Initialize();
		pModule->PostInitialize();
		pModule->m_IsInitialized = true;
	}

	void Engine::AddInternalModule(Module* pModule, bool initialize)
	{
		m_pInternalModules.push_back(pModule);
		m_pAllModules.push_back(pModule);
		if (!initialize) return;
		pModule->Initialize();
		pModule->PostInitialize();
		pModule->m_IsInitialized = true;
	}

	ScriptingExtender* Engine::GetScriptingExtender() const
	{
		return m_pScriptingExtender;
	}

	Module* Engine::GetMainModule(const std::type_info& type) const
	{
		auto it = std::find_if(m_pMainModules.begin(), m_pMainModules.end(), [&](Module* pModule)
		{
			return pModule->GetModuleType() == type;
		});

		if (it == m_pMainModules.end()) return nullptr;
		return *it;
	}

	Module* Engine::GetMainModule(const std::string& name) const
	{
		auto it = std::find_if(m_pMainModules.begin(), m_pMainModules.end(), [&](Module* pModule)
		{
			return pModule->GetMetaData().Name() == name;
		});

		if (it == m_pMainModules.end()) return nullptr;
		return *it;
	}

	Module* Engine::GetOptionalModule(const std::type_info& type) const
	{
		auto it = std::find_if(m_pOptionalModules.begin(), m_pOptionalModules.end(), [&](Module* pModule)
		{
			return pModule->GetModuleType() == type;
		});

		if (it == m_pOptionalModules.end()) return nullptr;
		return *it;
	}

	Module* Engine::GetOptionalModule(const std::string& name) const
	{
		auto it = std::find_if(m_pOptionalModules.begin(), m_pOptionalModules.end(), [&](Module* pModule)
		{
			return pModule->GetMetaData().Name() == name;
		});

		if (it == m_pOptionalModules.end()) return nullptr;
		return *it;
	}

	Module* Engine::GetModule(const std::type_info& type) const
	{
		auto it = std::find_if(m_pAllModules.begin(), m_pAllModules.end(), [&](Module* pModule)
		{
			return pModule->GetModuleType() == type;
		});

		if (it == m_pAllModules.end()) return nullptr;
		return *it;
	}

	Module* Engine::GetModule(const std::string& name) const
	{
		auto it = std::find_if(m_pAllModules.begin(), m_pAllModules.end(), [&](Module* pModule)
		{
			return pModule->GetMetaData().Name() == name;
		});

		if (it == m_pAllModules.end()) return nullptr;
		return *it;
	}

	Module* Engine::GetInternalModule(const std::type_info& type) const
	{
		auto it = std::find_if(m_pInternalModules.begin(), m_pInternalModules.end(), [&](Module* pModule)
		{
			return pModule->GetModuleType() == type;
		});

		if (it == m_pInternalModules.end()) return nullptr;
		return *it;
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

	GraphicsThread* Engine::GetGraphicsThread() const
	{
		return m_pGraphicsThread;
	}

	void Engine::StartThreads()
	{
		m_pGraphicsThread->Start();
	}

	void Engine::UpdateSceneManager()
	{
		m_pSceneManager->Update();
	}

	void Engine::DrawSceneManager()
	{
		m_pSceneManager->Draw();
	}

	Engine::Engine(const EngineCreateInfo& createInfo)
		: m_pSceneManager(new SceneManager(this)), m_pThreadManager(ThreadManager::GetInstance()),
		m_pJobManager(Jobs::JobManager::GetInstance()), m_pGraphicsThread(nullptr),
		m_pScriptingExtender(new ScriptingExtender()), m_CreateInfo(createInfo)
	{
		/* Copy main modules */
		m_pMainModules.resize(createInfo.MainModuleCount);
		m_pAllModules.resize(createInfo.MainModuleCount);
		std::memcpy(m_pMainModules.data(), createInfo.pMainModules, sizeof(Module*)*createInfo.MainModuleCount);
		std::memcpy(m_pAllModules.data(), createInfo.pMainModules, sizeof(Module*)*createInfo.MainModuleCount);

		/* Copy the optional modules into the optional modules vector */
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

		/* Add optional modules */
		size_t currentSize = m_pAllModules.size();
		m_pAllModules.resize(currentSize + m_pOptionalModules.size());
		for (size_t i = 0; i < m_pOptionalModules.size(); i++)
		{
			m_pAllModules[currentSize + i] = m_pOptionalModules[i];
		}

		m_pScriptingModules.resize(createInfo.ScriptingModulesCount);
		if (m_pScriptingModules.size() > 0) memcpy(&m_pScriptingModules[0], createInfo.pScriptingModules, createInfo.ScriptingModulesCount * sizeof(ScriptingModule*));

		/* Add scripting modules */
		currentSize = m_pAllModules.size();
		m_pAllModules.resize(currentSize + m_pScriptingModules.size() * 2);
		for (size_t i = 0; i < m_pScriptingModules.size(); ++i)
		{
			m_pAllModules[currentSize + i] = m_pScriptingModules[i];
			m_pAllModules[currentSize + m_pScriptingModules.size() + i] = m_pScriptingModules[i]->CreateLoaderModule();
		}

		AddInternalModule(new TimerModule);
		AddInternalModule(new ProfilerModule);
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

		m_pSceneManager->Cleanup();
		delete m_pSceneManager;
		m_pSceneManager = nullptr;

		Serializer::Cleanup();
		PropertySerializer::Cleanup();
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
			m_pPriorityInitializationModules[i]->m_IsInitialized = true;
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

			if (m_pAllModules[i]->m_IsInitialized) continue;			
			m_pAllModules[i]->m_pEngine = this;
			m_pAllModules[i]->Initialize();
			m_pAllModules[i]->m_IsInitialized = true;
		}

		AssetManager::Initialize();

		m_pScriptingExtender->Initialize(this);

		/* Create graphics thread */
		m_pGraphicsThread = new GraphicsThread(this);

		/* Run Post Initialize */
		for (size_t i = 0; i < m_pAllModules.size(); i++)
		{
			m_pAllModules[i]->PostInitialize();
		}
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
		STANDARD_SERIALIZER(glm::vec2);
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
		PropertySerializer::RegisterSerializer<ShapePropertySerializer>();
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
		ResourceType::RegisterType<ShapeProperty>();
		ResourceType::RegisterResource<SceneObject>("");
		ResourceType::RegisterResource<PrefabData>("");

		Reflect::RegisterBasicType<glm::vec2>("vec2");
		Reflect::RegisterBasicType<glm::vec3>("vec3");
		Reflect::RegisterBasicType<glm::vec4>("vec4");
		Reflect::RegisterBasicType<glm::quat>("quat");

		Reflect::RegisterBasicType<UUID>("UUID");
		Reflect::RegisterBasicType<LayerRef>("LayerRef");
		Reflect::RegisterBasicType<LayerMask>();
		Reflect::RegisterEnum<InputDeviceType>();
		Reflect::RegisterEnum<InputMappingType>();
		Reflect::RegisterEnum<InputState>();
		Reflect::RegisterEnum<AxisBlending>();

		Reflect::RegisterEnum<Filter>();
		Reflect::RegisterEnum<SamplerAddressMode>();
		Reflect::RegisterEnum<CompareOp>();

		/* Physics types */
		Reflect::RegisterEnum<BodyType>();
		Reflect::RegisterEnum<ShapeType>();
		Reflect::RegisterType<Box>();
		Reflect::RegisterType<Sphere>();
		Reflect::RegisterType<Cylinder>();
		Reflect::RegisterType<Capsule>();
		Reflect::RegisterType<TaperedCapsule>();
		Reflect::RegisterType<ShapeProperty>();

		Reflect::RegisterTemplatedType("AssetReference,Glory::AssetReference,class Glory::AssetReference", ST_Asset, sizeof(UUID));
	}

	void Engine::Update()
	{
		GameThreadFrameStart();
		Console::Update();
		m_pSceneManager->Update();
		m_pSceneManager->Draw();
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
