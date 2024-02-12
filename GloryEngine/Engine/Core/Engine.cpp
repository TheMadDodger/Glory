#include "Engine.h"
#include "Console.h"
#include "Resources.h"
#include "AssetLoader.h"
#include "PropertySerializer.h"
#include "AssetReferencePropertySerializer.h"
#include "ArrayPropertySerializer.h"
#include "EnumPropertySerializer.h"
#include "StructPropertySerializer.h"
#include "SceneObjectRefSerializer.h"
#include "ShapePropertySerializer.h"
#include "ShaderManager.h"
#include "LayerRef.h"
#include "SceneObjectRef.h"
#include "ShapeProperty.h"
#include "SceneManager.h"
#include "WindowModule.h"
#include "EngineProfiler.h"

#include "Debug.h"
#include "Console.h"
#include "AssetDatabase.h"
#include "Serializers.h"
#include "DisplayManager.h"
#include "LayerManager.h"
#include "ObjectManager.h"
#include "CameraManager.h"
#include "ShaderManager.h"
#include "GameTime.h"
#include "InternalResource.h"

#include "IModuleLoopHandler.h"
#include "GraphicsThread.h"
#include "ResourceLoaderModule.h"

#include "TimerModule.h"
#include "ProfilerModule.h"
#include "MaterialInstanceData.h"

#include <JobManager.h>
#include <ThreadManager.h>

#include <algorithm>

namespace Glory
{
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

	void Engine::AddLoaderModule(LoaderModule* pModule, bool initialize)
	{
		m_pLoaderModules.push_back(pModule);
		m_pAllModules.push_back(pModule);
		if (!initialize) return;
		pModule->Initialize();
		pModule->PostInitialize();
		pModule->m_IsInitialized = true;
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
		const ResourceType* pResourceType = m_ResourceTypes->GetResourceType(extension);
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
		m_pJobManager(Jobs::JobManager::GetInstance()), m_pGraphicsThread(nullptr), m_Reflection(new Reflect),
		m_CreateInfo(createInfo), m_ResourceTypes(new ResourceTypes),
		m_Time(new GameTime(this)), m_Debug(createInfo.m_pDebug), m_LayerManager(new LayerManager(this)),
		m_Console(createInfo.m_pConsole), m_Profiler(new EngineProfiler()),
		m_Serializers(new Serializers), m_CameraManager(new CameraManager(this)), m_DisplayManager(new DisplayManager),
		m_pShaderManager(createInfo.pShaderManager), m_pMaterialManager(createInfo.pMaterialManager),
		m_pResources(new Resources()), m_pAssetLoader(new AssetLoader(this)), m_ObjectManager(new ObjectManager)
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

		AddInternalModule(new TimerModule);
		ProfilerModule* pProfiler = new ProfilerModule();
		m_Profiler->m_pProfiler = pProfiler;
		AddInternalModule(pProfiler);
	}

	Engine::~Engine()
	{
		Cleanup();
	}

	void Engine::Initialize()
	{
		RegisterBasicTypes();
		RegisterStandardSerializers();
		m_pSceneManager->Initialize();

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
				uint32_t typeHash = ResourceTypes::GetHash(type);
				m_TypeHashToLoader[typeHash] = index;
			}

			if (m_pAllModules[i]->m_IsInitialized) continue;			
			m_pAllModules[i]->m_pEngine = this;
			m_pAllModules[i]->Initialize();
			m_pAllModules[i]->m_IsInitialized = true;
		}

		/* Create graphics thread */
		m_pGraphicsThread = new GraphicsThread(this);

		/* Run Post Initialize */
		for (size_t i = 0; i < m_pAllModules.size(); i++)
		{
			m_pAllModules[i]->PostInitialize();
		}
	}

	void Engine::Cleanup()
	{
		m_AssetDatabase->Destroy();
		m_pGraphicsThread->Stop();
		m_pJobManager->Kill();
		m_pThreadManager->Destroy();


		// We need to cleanup in reverse
		// This makes sure things like graphics get cleaned up before we close the window
		for (int i = (int)m_pAllModules.size() - 1; i >= 0; --i)
		{
			m_pAllModules[(size_t)i]->Cleanup();
			delete m_pAllModules[(size_t)i];
		}

		m_pSceneManager->Cleanup();

		delete m_pResources;
		m_pResources = nullptr;

		delete m_pAssetLoader;
		m_pAssetLoader = nullptr;

		delete m_pJobManager;
		m_pJobManager = nullptr;

		m_pAllModules.clear();
		m_pOptionalModules.clear();
		m_pPriorityInitializationModules.clear();
		m_TypeToLoader.clear();
		m_TypeHashToLoader.clear();
		m_pLoaderModules.clear();

		delete m_pGraphicsThread;
		m_pGraphicsThread = nullptr;

		delete m_pSceneManager;
		m_pSceneManager = nullptr;
	}

	GameTime& Engine::Time()
	{
		return *m_Time;
	}

	CameraManager& Engine::GetCameraManager()
	{
		return *m_CameraManager;
	}

	AssetDatabase& Engine::GetAssetDatabase()
	{
		return *m_AssetDatabase;
	}

	AssetManager& Engine::GetAssetManager()
	{
		return *m_AssetManager;
	}

	ResourceTypes& Engine::GetResourceTypes()
	{
		return *m_ResourceTypes;
	}

	Serializers& Engine::GetSerializers()
	{
		return *m_Serializers;
	}

	DisplayManager& Engine::GetDisplayManager()
	{
		return *m_DisplayManager;
	}

	Console& Engine::GetConsole()
	{
		return *m_Console;
	}

	LayerManager& Engine::GetLayerManager()
	{
		return *m_LayerManager;
	}

	ShaderManager& Engine::GetShaderManager()
	{
		return *m_pShaderManager;
	}

	MaterialManager& Engine::GetMaterialManager()
	{
		return *m_pMaterialManager;
	}

	Utils::Reflect::Reflect& Engine::Reflection()
	{
		return *m_Reflection;
	}

	ObjectManager& Engine::GetObjectManager()
	{
		return *m_ObjectManager;
	}

	EngineProfiler& Engine::Profiler()
	{
		return *m_Profiler;
	}

	void Engine::SetShaderManager(ShaderManager* pManager)
	{
		m_pShaderManager = pManager;
	}

	void Engine::SetMaterialManager(MaterialManager* pManager)
	{
		m_pMaterialManager = pManager;
	}

	Debug& Engine::GetDebug()
	{
		return *m_Debug;
	}

	void Engine::AddUserContext(uint32_t hash, void* pUserContext)
	{
		m_pUserContexts.emplace(hash, pUserContext);
	}

	void* Engine::GetUserContext(uint32_t hash)
	{
		return m_pUserContexts.at(hash);
	}

	void Engine::RequestQuit()
	{
		m_Quit = true;
	}

	void Engine::CancelQuit()
	{
		m_Quit = false;
	}

	bool Engine::WantsToQuit() const
	{
		return m_Quit;
	}

	void Engine::RegisterStandardSerializers()
	{
		// Standard
		m_Serializers->RegisterSerializer<SimpleTemplatedPropertySerializer<int>>();
		m_Serializers->RegisterSerializer<SimpleTemplatedPropertySerializer<float>>();
		m_Serializers->RegisterSerializer<SimpleTemplatedPropertySerializer<double>>();
		m_Serializers->RegisterSerializer<SimpleTemplatedPropertySerializer<bool>>();
		m_Serializers->RegisterSerializer<SimpleTemplatedPropertySerializer<long>>();
		m_Serializers->RegisterSerializer<SimpleTemplatedPropertySerializer<uint32_t>>();
		m_Serializers->RegisterSerializer<SimpleTemplatedPropertySerializer<uint64_t>>();
		m_Serializers->RegisterSerializer<SimpleTemplatedPropertySerializer<glm::vec2>>();
		m_Serializers->RegisterSerializer<SimpleTemplatedPropertySerializer<glm::vec3>>();
		m_Serializers->RegisterSerializer<SimpleTemplatedPropertySerializer<glm::vec4>>();
		m_Serializers->RegisterSerializer<SimpleTemplatedPropertySerializer<glm::quat>>();
		m_Serializers->RegisterSerializer<SimpleTemplatedPropertySerializer<LayerMask>>();
		m_Serializers->RegisterSerializer<SimpleTemplatedPropertySerializer<LayerRef>>();
		m_Serializers->RegisterSerializer<SimpleTemplatedPropertySerializer<SceneObjectRef>>();

		// Special
		m_Serializers->RegisterSerializer<AssetReferencePropertySerializer>();
		m_Serializers->RegisterSerializer<ArrayPropertySerializer>();
		m_Serializers->RegisterSerializer<EnumPropertySerializer>();
		m_Serializers->RegisterSerializer<StructPropertySerializer>();
		m_Serializers->RegisterSerializer<SceneObjectRefSerializer>();
		m_Serializers->RegisterSerializer<ShapePropertySerializer>();
	}

	void Engine::RegisterBasicTypes()
	{
		Reflect::SetReflectInstance(m_Reflection.get());

		Reflect::RegisterBasicType<int8_t>();
		Reflect::RegisterBasicType<int16_t>();
		Reflect::RegisterBasicType<int32_t>();
		Reflect::RegisterBasicType<int64_t>();
		Reflect::RegisterBasicType<uint8_t>();
		Reflect::RegisterBasicType<uint16_t>();
		Reflect::RegisterBasicType<uint32_t>();
		Reflect::RegisterBasicType<uint64_t>();
		Reflect::RegisterBasicType<char>();
		Reflect::RegisterBasicType<bool>();
		Reflect::RegisterBasicType<float>();
		Reflect::RegisterBasicType<double>();
		Reflect::RegisterBasicType<long>();
		Reflect::RegisterBasicType<unsigned long>();

		Reflect::RegisterTemplatedType("std::vector,vector", (size_t)CustomTypeHash::Array, 0);

		m_ResourceTypes->RegisterType<int>();
		m_ResourceTypes->RegisterType<float>();
		m_ResourceTypes->RegisterType<double>();
		m_ResourceTypes->RegisterType<long>();
		m_ResourceTypes->RegisterType<glm::vec2>();
		m_ResourceTypes->RegisterType<glm::vec3>();
		m_ResourceTypes->RegisterType<glm::vec4>();
		m_ResourceTypes->RegisterType<glm::quat>();
		m_ResourceTypes->RegisterType<LayerMask>();
		m_ResourceTypes->RegisterType<SceneObjectRef>();
		m_ResourceTypes->RegisterType<ShapeProperty>();
		m_ResourceTypes->RegisterResource<PrefabData>("");
		m_ResourceTypes->RegisterResource<MaterialData>("");
		m_ResourceTypes->RegisterResource<MaterialInstanceData>("");

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

		/* Shape types */
		Reflect::RegisterEnum<ShapeType>();
		Reflect::RegisterType<Box>();
		Reflect::RegisterType<Sphere>();
		Reflect::RegisterType<Cylinder>();
		Reflect::RegisterType<Capsule>();
		Reflect::RegisterType<TaperedCapsule>();
		Reflect::RegisterType<ShapeProperty>();

		Reflect::RegisterTemplatedType("AssetReference,Glory::AssetReference,class Glory::AssetReference", ST_Asset, sizeof(UUID));

		m_pResources->Register<ImageData>();
		m_pResources->Register<TextureData>();
		m_pResources->Register<MaterialData>();
		m_pResources->Register<MaterialInstanceData>();
		m_pResources->Register<ModelData>();
		m_pResources->Register<MeshData>();
		m_pResources->Register<PrefabData>();
	}

	void Engine::Update()
	{
		GameThreadFrameStart();
		m_Console->Update();
		m_pSceneManager->Update();
		m_pSceneManager->Draw();
		WindowModule* pWindows = GetMainModule<WindowModule>();
		if (pWindows) pWindows->PollEvents();
		ModulesLoop();
		GameThreadFrameEnd();
	}

	void Engine::ModulesLoop(IModuleLoopHandler* pLoopHandler)
	{
		m_pAssetLoader->DumpLoadedArchives();

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

	Resources& Engine::GetResources()
	{
		return *m_pResources;
	}

	AssetLoader& Engine::GetAssetLoader()
	{
		return *m_pAssetLoader;
	}

	Jobs::JobManager& Engine::Jobs()
	{
		return *m_pJobManager;
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
