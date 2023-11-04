#include "GloryContext.h"
#include "SceneManager.h"
#include "AssetManager.h"
#include "AssetDatabase.h"
#include "ResourceType.h"
#include "Serializer.h"
#include "DisplayManager.h"
#include "Console.h"
#include "LayerManager.h"
#include "WindowsDebugConsole.h"
#include "ObjectManager.h"
#include "ShaderManager.h"
#include "DistributedRandom.h"

#include <Reflection.h>

namespace Glory
{
	GloryContext* GloryContext::m_pContext = nullptr;

	GloryContext* GloryContext::CreateContext()
	{
		if (m_pContext != nullptr) return m_pContext;
		m_pContext = new GloryContext();

		Console::Initialize();

		/* TODO: Do not register this console in the runtime unless enabled? */
		Console::RegisterConsole<WindowsDebugConsole>();

		return m_pContext;
	}

	void GloryContext::DestroyContext()
	{
		if (!m_pContext) return;
		delete m_pContext;
		m_pContext = nullptr;
	}

	void GloryContext::SetContext(GloryContext* pContext)
	{
		m_pContext = pContext;
		Reflect::SetReflectInstance(m_pContext->m_pReflection);
	}

	GloryContext* GloryContext::GetContext()
	{
		return m_pContext;
	}

	void GloryContext::Initialize()
	{
		m_Game = &Game::GetGame();
	}

	Game* GloryContext::GetGame()
	{
		return m_pContext->m_Game;
	}
	
	CameraManager* GloryContext::GetCameraManager()
	{
		return &m_pContext->m_CameraManager;
	}

	AssetDatabase* GloryContext::GetAssetDatabase()
	{
		return m_pContext->m_pAssetDatabase;
	}

	AssetManager* GloryContext::GetAssetManager()
	{
		return m_pContext->m_pAssetManager;
	}

	ResourceTypes* GloryContext::GetResourceTypes()
	{
		return m_pContext->m_pResourceTypes;
	}

	Serializers* GloryContext::GetSerializers()
	{
		return m_pContext->m_pSerializers;
	}

	DisplayManager* GloryContext::GetDisplayManager()
	{
		return m_pContext->m_pDisplayManager;
	}

	Console* GloryContext::GetConsole()
	{
		return m_pContext->m_pConsole;
	}

	LayerManager* GloryContext::GetLayerManager()
	{
		return m_pContext->m_pLayerManager;
	}

	ShaderManager* GloryContext::GetShaderManager()
	{
		return m_pContext->m_pShaderManager;
	}

	Debug* GloryContext::GetDebug()
	{
		return m_pContext->m_pDebug;
	}

	void GloryContext::AddUserContext(uint32_t hash, void* pUserContext)
	{
		m_pContext->m_pUserContexts.emplace(hash, pUserContext);
	}

	void* GloryContext::GetUserContext(uint32_t hash)
	{
		return m_pContext->m_pUserContexts[hash];
	}

	Glory::GloryContext::GloryContext()
		: m_Game(nullptr), m_pAssetDatabase(new AssetDatabase()), m_pAssetManager(new AssetManager()), m_pResourceTypes(new ResourceTypes()),
		m_pSerializers(new Serializers()), m_pDisplayManager(new DisplayManager()), m_pConsole(new Console()),
		m_pLayerManager(new LayerManager()), m_pShaderManager(new ShaderManager()), m_pReflection(Reflect::CreateReflectInstance()),
		m_pDebug(new Debug()), m_pObjectManager(new ObjectManager()), m_UUIDRemapper(RandomDevice::Seed())
	{
	}

	Glory::GloryContext::~GloryContext()
	{
		delete m_pAssetDatabase;
		m_pAssetDatabase = nullptr;
		delete m_pAssetManager;
		m_pAssetManager = nullptr;
		delete m_pResourceTypes;
		m_pResourceTypes = nullptr;
		delete m_pSerializers;
		m_pSerializers = nullptr;
		delete m_pDisplayManager;
		m_pDisplayManager = nullptr;
		delete m_pConsole;
		m_pConsole = nullptr;
		delete m_pLayerManager;
		m_pLayerManager = nullptr;
		delete m_pShaderManager;
		m_pShaderManager = nullptr;
		delete m_pDebug;
		m_pDebug = nullptr;
		delete m_pObjectManager;
		m_pObjectManager = nullptr;

		m_CameraManager.Cleanup();
		Reflect::DestroyReflectInstance();
		m_pReflection = nullptr;
		m_Game = nullptr;
	}
}
