#pragma once
#include "HubWindow.h"

namespace Glory::EditorLauncher
{
	class ImGuiImpl
	{
	public:
		ImGuiImpl(HubWindow* pHubWindo);
		virtual ~ImGuiImpl();

		bool Initialize();

		HubWindow* GetHubWindow();
		void NewFrame();
		void Render();

	private:
		HubWindow* m_pHubWindow;
	};
}
