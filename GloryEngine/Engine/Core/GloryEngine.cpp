#include "GloryEngine.h"
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
#include "LightData.h"
#include "Input.h"

#include "PipelineData.h"
#include "MaterialData.h"
#include "PrefabData.h"
#include "AudioData.h"
#include "FontData.h"
#include "CubemapData.h"
#include "TextFileData.h"

#include "Debug.h"
#include "AssetDatabase.h"
#include "Serializers.h"
#include "LayerManager.h"
#include "ObjectManager.h"
#include "CameraManager.h"
#include "GameTime.h"
#include "BinaryStream.h"
#include "RenderData.h"
#include "GraphicsDevice.h"
#include "Renderer.h"

#include "IModuleLoopHandler.h"
#include "ResourceLoaderModule.h"

#include <JobManager.h>
#include <ThreadManager.h>

#include <algorithm>

namespace Glory
{
	SceneManager* GloryEngine::GetSceneManager()
	{
		return m_pSceneManager;
	}

	void GloryEngine::AddMainModule(Module* pModule, bool initialize)
	{
		m_pMainModules.push_back(pModule);
		m_pAllModules.push_back(pModule);
		if (!initialize) return;
		pModule->Initialize();
		pModule->PostInitialize();
		pModule->m_IsInitialized = true;
	}

	void GloryEngine::AddOptionalModule(Module* pModule, bool initialize)
	{
		m_pOptionalModules.push_back(pModule);
		m_pAllModules.push_back(pModule);
		if (!initialize) return;
		pModule->Initialize();
		pModule->PostInitialize();
		pModule->m_IsInitialized = true;
	}

	void GloryEngine::AddInternalModule(Module* pModule, bool initialize)
	{
		m_pInternalModules.push_back(pModule);
		m_pAllModules.push_back(pModule);
		if (!initialize) return;
		pModule->Initialize();
		pModule->PostInitialize();
		pModule->m_IsInitialized = true;
	}

	void GloryEngine::AddLoaderModule(LoaderModule* pModule, bool initialize)
	{
		m_pLoaderModules.push_back(pModule);
		m_pAllModules.push_back(pModule);
		if (!initialize) return;
		pModule->Initialize();
		pModule->PostInitialize();
		pModule->m_IsInitialized = true;
	}

	Module* GloryEngine::GetMainModule(const std::type_info& type) const
	{
		auto it = std::find_if(m_pMainModules.begin(), m_pMainModules.end(), [&](Module* pModule)
		{
			return pModule->GetModuleType() == type || pModule->GetBaseModuleType() == type;
		});

		if (it == m_pMainModules.end()) return nullptr;
		return *it;
	}

	Module* GloryEngine::GetMainModule(const std::string& name) const
	{
		auto it = std::find_if(m_pMainModules.begin(), m_pMainModules.end(), [&](Module* pModule)
		{
			return pModule->GetMetaData().Name() == name;
		});

		if (it == m_pMainModules.end()) return nullptr;
		return *it;
	}

	Module* GloryEngine::GetOptionalModule(const std::type_info& type) const
	{
		auto it = std::find_if(m_pOptionalModules.begin(), m_pOptionalModules.end(), [&](Module* pModule)
		{
			return pModule->GetModuleType() == type || pModule->GetBaseModuleType() == type;
		});

		if (it == m_pOptionalModules.end()) return nullptr;
		return *it;
	}

	Module* GloryEngine::GetOptionalModule(const std::string& name) const
	{
		auto it = std::find_if(m_pOptionalModules.begin(), m_pOptionalModules.end(), [&](Module* pModule)
		{
			return pModule->GetMetaData().Name() == name;
		});

		if (it == m_pOptionalModules.end()) return nullptr;
		return *it;
	}

	Module* GloryEngine::GetModule(const std::type_info& type) const
	{
		auto it = std::find_if(m_pAllModules.begin(), m_pAllModules.end(), [&](Module* pModule)
		{
			return pModule->GetModuleType() == type || pModule->GetBaseModuleType() == type;
		});

		if (it == m_pAllModules.end()) return nullptr;
		return *it;
	}

	Module* GloryEngine::GetModule(const std::string& name) const
	{
		auto it = std::find_if(m_pAllModules.begin(), m_pAllModules.end(), [&](Module* pModule)
		{
			return pModule->GetMetaData().Name() == name;
		});

		if (it == m_pAllModules.end()) return nullptr;
		return *it;
	}

	Module* GloryEngine::GetInternalModule(const std::type_info& type) const
	{
		auto it = std::find_if(m_pInternalModules.begin(), m_pInternalModules.end(), [&](Module* pModule)
		{
			return pModule->GetModuleType() == type || pModule->GetBaseModuleType() == type;
		});

		if (it == m_pInternalModules.end()) return nullptr;
		return *it;
	}

	LoaderModule* GloryEngine::GetLoaderModule(const std::string& extension)
	{
		const ResourceType* pResourceType = m_ResourceTypes->GetResourceType(extension);
		if (!pResourceType) return nullptr;
		return GetLoaderModule(pResourceType->Hash());
	}

	LoaderModule* GloryEngine::GetLoaderModule(const std::type_info& resourceType)
	{
		if (m_TypeToLoader.find(resourceType) == m_TypeToLoader.end()) return nullptr;
		size_t loaderIndex = m_TypeToLoader[resourceType];
		return m_pLoaderModules[loaderIndex];
	}

	LoaderModule* GloryEngine::GetLoaderModule(uint32_t typeHash)
	{
		if (m_TypeHashToLoader.find(typeHash) == m_TypeHashToLoader.end()) return nullptr;
		size_t loaderIndex = m_TypeHashToLoader[typeHash];
		return m_pLoaderModules[loaderIndex];
	}

	void GloryEngine::UpdateSceneManager()
	{
		m_pSceneManager->Update();
	}

	void GloryEngine::DrawSceneManager()
	{
		m_pSceneManager->SetRenderer(ActiveRenderer());
		m_pSceneManager->Draw();
	}

	GloryEngine::GloryEngine(const EngineCreateInfo& createInfo)
		: m_ActiveGraphicsDevice(0), m_ActiveRenderer(0), m_pSceneManager(createInfo.pSceneManager),
		m_ThreadManager(new ThreadManager()), m_JobManager(new Jobs::JobManager(m_ThreadManager.get())),
		m_Reflection(new Reflect), m_CreateInfo(createInfo), m_ResourceTypes(new ResourceTypes),
		m_Time(new GameTime(this)), m_Debug(createInfo.m_pDebug), m_LayerManager(new LayerManager(this)),
		m_pAssetsManager(createInfo.pAssetManager), m_Console(createInfo.m_pConsole), m_Profiler(new EngineProfiler()),
		m_Serializers(new Serializers(this)), m_CameraManager(new CameraManager(this)),
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
	}

	GloryEngine::~GloryEngine()
	{
		Cleanup();
	}

	void GloryEngine::Initialize()
	{
		if (m_Initialized) return;

		m_Console->Initialize();
		m_UUIDRemapper.Reset();

		WindowModule* pWindows = IEngine::GetMainModule<WindowModule>();
		m_Debug->SetWindowModule(pWindows);

		RegisterBasicTypes();
		RegisterStandardSerializers();
		m_pSceneManager->Initialize();

		for (size_t i = 0; i < m_pAllModules.size(); i++)
		{
			if (m_pAllModules[i]->m_IsInitialized) continue;
			m_pAllModules[i]->m_pEngine = this;
			m_pAllModules[i]->PreInitialize();
		}

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

		/* Initialize renderers */
		for (size_t i = 0; i < m_pRenderers.size(); ++i)
		{
			m_pRenderers[i]->Initialize();
		}

		m_pSceneManager->SetRenderer(ActiveRenderer());

		m_Console->RegisterCommand(new ConsoleCommand1<size_t>("type", [this](size_t hash) {
			const Utils::Reflect::TypeData* pType = Reflect::GetTyeData(hash);
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

	void GloryEngine::Cleanup()
	{
		if (!m_Initialized) return;

		for (Renderer* pRenderer : m_pRenderers)
			pRenderer->Cleanup();
		for (Renderer* pRenderer : m_pSecondaryRenderers)
			pRenderer->Cleanup();

		m_Console->Cleanup();
		m_AssetDatabase->Destroy();
		m_JobManager->Kill();
		m_ThreadManager->Kill();

		// We need to cleanup in reverse
		// This makes sure things like graphics get cleaned up before we close the window
		for (int i = (int)m_pAllModules.size() - 1; i >= 0; --i)
		{
			m_pAllModules[(size_t)i]->Cleanup();
			delete m_pAllModules[(size_t)i];
		}

		m_pSceneManager = nullptr;

		m_pAllModules.clear();
		m_pOptionalModules.clear();
		m_pPriorityInitializationModules.clear();
		m_TypeToLoader.clear();
		m_TypeHashToLoader.clear();
		m_pLoaderModules.clear();
		m_pRenderers.clear();
		m_pSecondaryRenderers.clear();

		m_Initialized = false;
	}

	void GloryEngine::Draw()
	{
		if (!m_pRenderers.empty())
		{
			GetDebug().SubmitLines(m_pRenderers[m_ActiveRenderer], &Time());
			m_pRenderers[m_ActiveRenderer]->Draw();
		}

		for (Renderer* pRenderer : m_pSecondaryRenderers)
			pRenderer->Draw();
	}

	GameTime& GloryEngine::Time()
	{
		return *m_Time;
	}

	CameraManager& GloryEngine::GetCameraManager()
	{
		return *m_CameraManager;
	}

	AssetDatabase& GloryEngine::GetAssetDatabase()
	{
		return *m_AssetDatabase;
	}

	AssetManager& GloryEngine::GetAssetManager()
	{
		return *m_pAssetsManager;
	}

	ResourceTypes& GloryEngine::GetResourceTypes()
	{
		return *m_ResourceTypes;
	}

	Serializers& GloryEngine::GetSerializers()
	{
		return *m_Serializers;
	}

	Console& GloryEngine::GetConsole()
	{
		return *m_Console;
	}

	LayerManager& GloryEngine::GetLayerManager()
	{
		return *m_LayerManager;
	}

	MaterialManager& GloryEngine::GetMaterialManager()
	{
		return *m_pMaterialManager;
	}

	PipelineManager& GloryEngine::GetPipelineManager()
	{
		return *m_pPipelineManager;
	}

	Utils::Reflect::Reflect& GloryEngine::Reflection()
	{
		return *m_Reflection;
	}

	ObjectManager& GloryEngine::GetObjectManager()
	{
		return *m_ObjectManager;
	}

	EngineProfiler& GloryEngine::Profiler()
	{
		return *m_Profiler;
	}

	ThreadManager& GloryEngine::Threads()
	{
		return *m_ThreadManager;
	}

	Jobs::JobManager& GloryEngine::Jobs()
	{
		return *m_JobManager;
	}

	void GloryEngine::SetAssetManager(AssetManager* pManager)
	{
		m_pAssetsManager = pManager;
	}

	void GloryEngine::SetSceneManager(SceneManager* pManager)
	{
		m_pSceneManager = pManager;
	}

	void GloryEngine::SetMaterialManager(MaterialManager* pManager)
	{
		m_pMaterialManager = pManager;
	}

	void GloryEngine::SetPipelineManager(PipelineManager* pManager)
	{
		m_pPipelineManager = pManager;
	}

	Debug& GloryEngine::GetDebug()
	{
		return *m_Debug;
	}

	void GloryEngine::AddUserContext(uint32_t hash, void* pUserContext)
	{
		m_pUserContexts.emplace(hash, pUserContext);
	}

	void* GloryEngine::GetUserContext(uint32_t hash)
	{
		return m_pUserContexts.at(hash);
	}

	void GloryEngine::RequestQuit()
	{
		m_Quit = true;
	}

	void GloryEngine::CancelQuit()
	{
		m_Quit = false;
	}

	bool GloryEngine::WantsToQuit() const
	{
		return m_Quit;
	}

	void GloryEngine::AddData(const std::filesystem::path& path, const std::string& name, std::vector<char>&& data)
	{
		m_Datas.emplace(name, std::move(data));
		m_DataPaths.emplace(name, path);
	}

	void GloryEngine::ProcessData()
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

		if (HasData("General"))
		{
			BinaryMemoryStream memoryStream{ GetData("General") };
			BinaryStream* stream = &memoryStream;
			stream->Read(m_ApplicationVersion);
			stream->Read(m_Organization).Read(m_AppName);
		}

		if (HasData("Renderer"))
		{
			//m_PipelineOrder.clear();
			//
			//std::vector<char> buffer = m_pEngine->GetData("Renderer");
			//
			//BinaryMemoryStream memoryStream{ buffer };
			//BinaryStream* stream = &memoryStream;
			//
			//size_t pipelineCount;
			//stream->Read(pipelineCount);
			//for (size_t i = 0; i < pipelineCount; ++i)
			//{
			//	UUID pipelineID;
			//	stream->Read(pipelineID);
			//	m_PipelineOrder.emplace_back(pipelineID);
			//}
		}

		for (size_t i = 0; i < m_pAllModules.size(); ++i)
		{
			m_pAllModules[i]->OnProcessData();
		}
	}

	bool GloryEngine::HasData(const std::string& name) const
	{
		return m_Datas.find(name) != m_Datas.end();
	}

	const std::filesystem::path& GloryEngine::DataPath(const std::string& name) const
	{
		return m_DataPaths.at(name);
	}

	std::vector<char>& GloryEngine::GetData(const std::string& name)
	{
		return m_Datas.at(name);
	}

	void GloryEngine::SetRootPath(const std::filesystem::path& path)
	{
		m_RootPath = path;
	}

	const std::filesystem::path& GloryEngine::RootPath() const
	{
		return m_RootPath;
	}

	void GloryEngine::SetApplicationVersion(uint32_t major, uint32_t minor, uint32_t subMinor, uint32_t rc)
	{
		m_ApplicationVersion.Major = (int)major;
		m_ApplicationVersion.Minor = (int)minor;
		m_ApplicationVersion.SubMinor = (int)subMinor;
		m_ApplicationVersion.RC = (int)rc;
	}

	const Version& GloryEngine::GetApplicationVersion() const
	{
		return m_ApplicationVersion;
	}

	void GloryEngine::SetOrganizationAndAppName(std::string&& organization, std::string&& appName)
	{
		m_Organization = std::move(organization);
		m_AppName = std::move(appName);
	}

	std::string_view GloryEngine::Organization() const
	{
		return m_Organization;
	}

	std::string_view GloryEngine::AppName() const
	{
		return m_AppName;
	}

	void GloryEngine::SetMainWindowInfo(WindowCreateInfo&& info)
	{
		m_MainWindowInfo = std::move(info);
	}

	WindowCreateInfo& GloryEngine::MainWindowInfo()
	{
		return m_MainWindowInfo;
	}

	void GloryEngine::AddGraphicsDevice(GraphicsDevice* pGraphicsDevice)
	{
		m_pGraphicsDevices.emplace_back(pGraphicsDevice);
		pGraphicsDevice->Initialize();
	}

	GraphicsDevice* GloryEngine::ActiveGraphicsDevice()
	{
		return m_ActiveGraphicsDevice >= m_pGraphicsDevices.size() ?
			nullptr : m_pGraphicsDevices[m_ActiveGraphicsDevice];
	}

	void GloryEngine::AddMainRenderer(Renderer* pRenderer)
	{
		m_pRenderers.emplace_back(pRenderer);
		pRenderer->InitializeAsMainRenderer();
	}

	Renderer* GloryEngine::ActiveRenderer()
	{
		return m_pRenderers.empty() ? nullptr : m_pRenderers[m_ActiveRenderer];
	}

	void GloryEngine::Load()
	{
		for (Module* pModule : m_pAllModules)
			pModule->Preload();
		for (Module* pModule : m_pAllModules)
			pModule->Load();
		for (Module* pModule : m_pAllModules)
			pModule->Postload();
	}

	UUIDRemapper& GloryEngine::GetUUIDRemapper()
	{
		return m_UUIDRemapper;
	}

	void GloryEngine::RegisterStandardSerializers()
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

	void GloryEngine::RegisterBasicTypes()
	{
		Reflect::SetReflectInstance(m_Reflection.get());
		Utils::ECS::ComponentTypes::SetInstance(m_pSceneManager->ComponentTypesInstance());
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
		m_ResourceTypes->RegisterResource<MeshData>("");
		m_ResourceTypes->RegisterResource<ModelData>("");
		m_ResourceTypes->RegisterResource<ImageData>("");
		m_ResourceTypes->RegisterResource<AudioData>("");
		m_ResourceTypes->RegisterResource<FontData>("");
		m_ResourceTypes->RegisterResource<CubemapData>("");
		m_ResourceTypes->RegisterResource<TextFileData>("");

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
		Reflect::RegisterEnum<Func>();
		Reflect::RegisterEnum<PipelineType>();
		Reflect::RegisterEnum<BlurType>();
		Reflect::RegisterEnum<Alignment>();
		Reflect::RegisterEnum<LightType>();
		Reflect::RegisterEnum<CullFace>();
		Reflect::RegisterEnum<PrimitiveType>();
		Reflect::RegisterEnum<BlendFactor>();
		Reflect::RegisterEnum<BlendOp>();

		/* Shape types */
		Reflect::RegisterEnum<ShapeType>();
		Reflect::RegisterType<Box>();
		Reflect::RegisterType<Sphere>();
		Reflect::RegisterType<Cylinder>();
		Reflect::RegisterType<Capsule>();
		Reflect::RegisterType<TaperedCapsule>();
		Reflect::RegisterType<ShapeProperty>();

		Reflect::RegisterBasicType<TextureData>("TextureData");
		Reflect::RegisterBasicType<FontData>("FontData");
		Reflect::RegisterTemplatedType("AssetReference,Glory::AssetReference,class Glory::AssetReference", ST_Asset, sizeof(UUID));
	}

	void GloryEngine::Update()
	{
		BeginFrame();
		m_Console->Update();
		WindowModule* pWindows = IEngine::GetMainModule<WindowModule>();
		if (pWindows) pWindows->PollEvents();
		m_pSceneManager->SetRenderer(ActiveRenderer());
		m_pSceneManager->Update();
		m_pSceneManager->Draw();
		ModulesLoop();
		Draw();
		EndFrame();
	}

	void GloryEngine::ModulesLoop(IModuleLoopHandler* pLoopHandler)
	{
		for (size_t i = 0; i < m_pAllModules.size(); i++)
		{
			Module* pModule = m_pAllModules[i];

			if (pLoopHandler != nullptr && pLoopHandler->HandleModuleLoop(pModule)) continue;

			m_pAllModules[i]->Update();
			m_pAllModules[i]->Draw();
		}
	}

	void GloryEngine::BeginFrame()
	{
		m_Profiler->BeginThread("Main");
		m_Time->BeginFrame();

		GraphicsDevice* pDevice = ActiveGraphicsDevice();
		if (pDevice) pDevice->BeginFrame();

		for (size_t i = 0; i < m_pAllModules.size(); i++)
		{
			m_pAllModules[i]->OnBeginFrame();
		}

		if (!m_pRenderers.empty())
			m_pRenderers[m_ActiveRenderer]->BeginFrame();

		for (Renderer* pRenderer : m_pSecondaryRenderers)
			pRenderer->BeginFrame();
	}

	void GloryEngine::EndFrame()
	{
		for (size_t i = 0; i < m_pAllModules.size(); i++)
		{
			m_pAllModules[i]->OnEndFrame();
		}

		if (!m_pRenderers.empty())
			m_pRenderers[m_ActiveRenderer]->EndFrame();

		for (Renderer* pRenderer : m_pSecondaryRenderers)
			pRenderer->EndFrame();

		GraphicsDevice* pDevice = ActiveGraphicsDevice();
		if (pDevice) pDevice->EndFrame();

		m_Time->EndFrame();
		m_Profiler->EndThread();
	}

	void GloryEngine::CallModuleUpdate(Module* pModule) const
	{
		pModule->Update();
	}

	void GloryEngine::CallModuleDraw(Module* pModule) const
	{
		pModule->Draw();
	}

	const size_t GloryEngine::InternalModulesCount() const
	{
		return m_pInternalModules.size();
	}

	const size_t GloryEngine::ModulesCount() const
	{
		return m_pAllModules.size();
	}

	Module* GloryEngine::GetModule(size_t index) const
	{
		if (index >= m_pAllModules.size()) return nullptr;
		return m_pAllModules[index];
	}

	void GloryEngine::LoadModuleSettings(const std::filesystem::path& overrideRootPath)
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
