#pragma once
#include "Game.h"

namespace Glory
{
	class GloryContext
	{
	public:
		static GloryContext* CreateContext();
		static void SetContext(GloryContext* pContext);
		static GloryContext* GetContext();
		void Initialize();

		static Game* GetGame();

	private:
		GloryContext();
		virtual ~GloryContext();

	private:
		static GloryContext* m_pContext;
		Game* m_Game;
	};
}