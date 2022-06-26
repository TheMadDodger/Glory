#include "GloryContext.h"

namespace Glory
{
	GloryContext* GloryContext::m_pContext = nullptr;

	GloryContext* GloryContext::CreateContext()
	{
		if (m_pContext != nullptr) return m_pContext;
		m_pContext = new GloryContext();
		return m_pContext;
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

	Glory::GloryContext::GloryContext() : m_Game(nullptr)
	{
	}

	Glory::GloryContext::~GloryContext()
	{
	}
}
