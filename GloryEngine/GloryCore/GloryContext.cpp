#include "GloryContext.h"
#include "AssetManager.h"
#include "AssetDatabase.h"

namespace Glory
{
	GloryContext* GloryContext::m_pContext = nullptr;

	GloryContext* GloryContext::CreateContext()
	{
		if (m_pContext != nullptr) return m_pContext;
		m_pContext = new GloryContext();
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

	Glory::GloryContext::GloryContext() : m_Game(nullptr), m_pAssetDatabase(new AssetDatabase()), m_pAssetManager(new AssetManager())
	{
	}

	Glory::GloryContext::~GloryContext()
	{
		delete m_pAssetDatabase;
		m_pAssetDatabase = nullptr;
		delete m_pAssetManager;
		m_pAssetManager = nullptr;
	}
}
