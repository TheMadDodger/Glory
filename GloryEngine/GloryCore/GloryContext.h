#pragma once
#include "Game.h"
#include "CameraManager.h"

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
		static CameraManager* GetCameraManager();

	private:
		GloryContext();
		virtual ~GloryContext();

	private:
		static GloryContext* m_pContext;
		Game* m_Game;
		CameraManager m_CameraManager;
	};
}