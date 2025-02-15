#include "Engine.h"
#include "Console.h"
#include "AssetManager.h"
#include "PropertySerializer.h"
#include "AssetReferencePropertySerializer.h"
#include "ArrayPropertySerializer.h"
#include "EnumPropertySerializer.h"
#include "StructPropertySerializer.h"
#include "SceneObjectRefSerializer.h"
#include "ShapePropertySerializer.h"
#include "LayerRef.h"
#include "SceneObjectRef.h"
#include "ShapeProperty.h"
#include "SceneManager.h"
#include "WindowModule.h"
#include "EngineProfiler.h"

#include "MaterialInstanceData.h"
#include "PipelineData.h"
#include "PrefabData.h"
#include "AudioData.h"
#include "FontData.h"

#include "Debug.h"
#include "Console.h"
#include "AssetDatabase.h"
#include "AssetManager.h"
#include "Serializers.h"
#include "DisplayManager.h"
#include "LayerManager.h"
#include "ObjectManager.h"
#include "CameraManager.h"
#include "GameTime.h"
#include "BinaryStream.h"
#include "RenderData.h"

#include "IModuleLoopHandler.h"
#include "ResourceLoaderModule.h"

#include "ProfilerModule.h"

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
			return pModule->GetModuleType() == type || pModule->GetBaseModuleType() == type;
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
			return pModule->GetModuleType() == type || pModule->GetBaseModuleType() == type;
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
			return pModule->GetModuleType() == type || pModule->GetBaseModuleType() == type;
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
			return pModule->GetModuleType() == type || pModule->GetBaseModuleType() == type;
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

	void Engine::UpdateSceneManager()
	{
		m_pSceneManager->Update();
	}

	void Engine::DrawSceneManager()
	{
		m_pSceneManager->Draw();
	}

	Engine::Engine(const EngineCreateInfo& createInfo)
		: m_pSceneManager(createInfo.pSceneManager), m_pThreadManager(ThreadManager::GetInstance()),
		m_pJobManager(Jobs::JobManager::GetInstance()), m_Reflection(new Reflect),
		m_CreateInfo(createInfo), m_ResourceTypes(new ResourceTypes),
		m_Time(new GameTime(this)), m_Debug(createInfo.m_pDebug), m_LayerManager(new LayerManager(this)),
		m_pAssetsManager(createInfo.pAssetManager), m_Console(createInfo.m_pConsole), m_Profiler(new EngineProfiler()),
		m_Serializers(new Serializers(this)), m_CameraManager(new CameraManager(this)), m_DisplayManager(new DisplayManager),
		m_pMaterialManager(createInfo.pMaterialManager), m_AssetDatabase(new AssetDatabase),
		m_ObjectManager(new ObjectManager), m_RootPath("./")
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
		if (m_Initialized) return;

		m_UUIDRemapper.Reset();

		WindowModule* pWindows = GetMainModule<WindowModule>();
		m_Debug->SetWindowModule(pWindows);

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

		m_AssetDatabase->Initialize();
		m_pAssetsManager->Initialize();

		/* Run Post Initialize */
		for (size_t i = 0; i < m_pAllModules.size(); i++)
		{
			m_pAllModules[i]->PostInitialize();
		}

		m_Console->RegisterCommand(new ConsoleCommand1<size_t>("type", [this](size_t hash) {
			const Utils::Reflect::TypeData* pType = m_Reflection->GetTyeData(hash);
			if (!pType) return false;
			m_Console->WriteLine(std::to_string(hash) + " = " + pType->TypeName());
			return true;
		}));

		m_Console->RegisterCommand(new ConsoleCommand1<size_t>("resourcetype", [this](size_t hash) {
			const ResourceType* pType = m_ResourceTypes->GetResourceType(hash);
			if (!pType) return false;
			m_Console->WriteLine("Resource type info for: " + std::to_string(hash));
			m_Console->WriteLine("Name: " + pType->Name());
			m_Console->WriteLine("Full name: " + pType->FullName());
			m_Console->WriteLine("Extensions (legacy): " + pType->Extensions());
			return true;
		}));

		m_Initialized = true;

		m_Time->Initialize();
	}

	void Engine::Cleanup()
	{
		if (!m_Initialized) return;

		m_AssetDatabase->Destroy();
		m_pJobManager->Kill();
		m_pThreadManager->Destroy();

		// We need to cleanup in reverse
		// This makes sure things like graphics get cleaned up before we close the window
		for (int i = (int)m_pAllModules.size() - 1; i >= 0; --i)
		{
			m_pAllModules[(size_t)i]->Cleanup();
			delete m_pAllModules[(size_t)i];
		}

		m_pSceneManager = nullptr;

		delete m_pJobManager;
		m_pJobManager = nullptr;

		m_pAllModules.clear();
		m_pOptionalModules.clear();
		m_pPriorityInitializationModules.clear();
		m_TypeToLoader.clear();
		m_TypeHashToLoader.clear();
		m_pLoaderModules.clear();

		m_Initialized = false;
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
		return *m_pAssetsManager;
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

	MaterialManager& Engine::GetMaterialManager()
	{
		return *m_pMaterialManager;
	}

	PipelineManager& Engine::GetPipelineManager()
	{
		return *m_pPipelineManager;
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

	Jobs::JobManager& Engine::Jobs()
	{
		return *m_pJobManager;
	}

	void Engine::SetAssetManager(AssetManager* pManager)
	{
		m_pAssetsManager = pManager;
	}

	void Engine::SetSceneManager(SceneManager* pManager)
	{
		m_pSceneManager = pManager;
	}

	void Engine::SetMaterialManager(MaterialManager* pManager)
	{
		m_pMaterialManager = pManager;
	}

	void Engine::SetPipelineManager(PipelineManager* pManager)
	{
		m_pPipelineManager = pManager;
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

	void Engine::AddData(const std::string& name, std::vector<char>&& data)
	{
		m_Datas.emplace(name, std::move(data));
	}

	void Engine::ProcessData()
	{
		if (HasData("Layers"))
		{
			BinaryMemoryStream memoryStream{ GetData("Layers") };
			BinaryStream* stream = &memoryStream;
			while (!stream->Eof())
			{
				std::string name;
				stream->Read(name);
				m_LayerManager->AddLayer(name);
			}
		}

		for (size_t i = 0; i < m_pAllModules.size(); ++i)
		{
			m_pAllModules[i]->OnProcessData();
		}
	}

	bool Engine::HasData(const std::string& name)
	{
		return m_Datas.find(name) != m_Datas.end();
	}

	std::vector<char>& Engine::GetData(const std::string& name)
	{
		return m_Datas.at(name);
	}

	void Engine::SetRootPath(const std::filesystem::path& path)
	{
		m_RootPath = path;
	}

	const std::filesystem::path& Engine::RootPath() const
	{
		return m_RootPath;
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
		m_Serializers->RegisterSerializer<SimpleTemplatedPropertySerializer<std::string>>();

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
		m_ResourceTypes->RegisterResource<PipelineData>("");
		m_ResourceTypes->RegisterResource<MaterialData>("");
		m_ResourceTypes->RegisterResource<MaterialInstanceData>("");
		m_ResourceTypes->RegisterResource<MeshData>("");
		m_ResourceTypes->RegisterResource<ModelData>("");
		m_ResourceTypes->RegisterResource<ImageData>("");
		m_ResourceTypes->RegisterResource<AudioData>("");
		m_ResourceTypes->RegisterResource<FontData>("");

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
		Reflect::RegisterEnum<PipelineType>();
		Reflect::RegisterEnum<BlurType>();
		Reflect::RegisterEnum<Alignment>();

		/* Shape types */
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
		BeginFrame();
		m_Console->Update();
		WindowModule* pWindows = GetMainModule<WindowModule>();
		if (pWindows) pWindows->PollEvents();
		m_pSceneManager->Update();
		m_pSceneManager->Draw();
		ModulesLoop();
		EndFrame();
	}

	void Engine::ModulesLoop(IModuleLoopHandler* pLoopHandler)
	{
		for (size_t i = 0; i < m_pAllModules.size(); i++)
		{
			Module* pModule = m_pAllModules[i];

			if (pLoopHandler != nullptr && pLoopHandler->HandleModuleLoop(pModule)) continue;

			m_pAllModules[i]->Update();
			m_pAllModules[i]->Draw();
		}
	}

	void Engine::BeginFrame()
	{
		m_Time->BeginFrame();
		for (size_t i = 0; i < m_pAllModules.size(); i++)
		{
			m_pAllModules[i]->OnBeginFrame();
		}
	}

	void Engine::EndFrame()
	{
		for (size_t i = 0; i < m_pAllModules.size(); i++)
		{
			m_pAllModules[i]->OnEndFrame();
		}
		m_Time->EndFrame();
	}

	void Engine::CallModuleUpdate(Module* pModule)
	{
		pModule->Update();
	}

	void Engine::CallModuleDraw(Module* pModule)
	{
		pModule->Draw();
	}

	const size_t Engine::InternalModulesCount() const
	{
		return m_pInternalModules.size();
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
}
