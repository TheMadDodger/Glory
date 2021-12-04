#pragma once
#include "Module.h"
#include <vector>
#include "RenderFrame.h"

namespace Glory
{
	class RendererModule : public Module
	{
	public:
		RendererModule();
		virtual ~RendererModule();

		virtual const std::type_info& GetModuleType() override;

		void Submit(const RenderData& renderData);
		void Submit(const CoreCamera& camera);

		void StartFrame();
		void EndFrame();

	protected:
		virtual void OnSubmit(const RenderData& renderData) {}
		virtual void OnSubmit(const CoreCamera& renderData) {}

	protected:
		friend class GraphicsThread;
		virtual void Initialize() = 0;
		virtual void Cleanup() = 0;
		virtual void Render(const RenderFrame& frame) = 0;
		
	private:
		void ThreadedInitialize();
		void ThreadedCleanup();

	private:
		RenderFrame m_CurrentPreparingFrame;
	};
}
