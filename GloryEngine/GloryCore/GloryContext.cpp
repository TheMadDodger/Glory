#include "GloryContext.h"
#include "AssetManager.h"
#include "AssetDatabase.h"
#include "ResourceType.h"
#include "Serializer.h"
#include "DisplayManager.h"
#include "SerializedPropertyManager.h"
#include "Console.h"
#include "WindowsDebugConsole.h"

namespace Glory
{
	GloryContext* GloryContext::m_pContext = nullptr;

	GloryContext* GloryContext::CreateContext()
	{
		if (m_pContext != nullptr) return m_pContext;
		m_pContext = new GloryContext();

		Console::Initialize();

#ifdef _DEBUG
		Console::RegisterConsole<WindowsDebugConsole>();
#endif
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

	SerializedPropertyManager* GloryContext::GetSerializedPropertyManager()
	{
		return m_pContext->m_pSerializedPropertyManager;
	}

	Console* GloryContext::GetConsole()
	{
		return m_pContext->m_pConsole;
	}

	Glory::GloryContext::GloryContext()
		: m_Game(nullptr), m_pAssetDatabase(new AssetDatabase()), m_pAssetManager(new AssetManager()), m_pResourceTypes(new ResourceTypes()),
		m_pSerializers(new Serializers()), m_pDisplayManager(new DisplayManager()), m_pSerializedPropertyManager(new SerializedPropertyManager()),
		m_pConsole(new Console())
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
		delete m_pSerializedPropertyManager;
		m_pSerializedPropertyManager = nullptr;
		delete m_pConsole;
		m_pConsole = nullptr;

		m_CameraManager.Cleanup();
	}
}
