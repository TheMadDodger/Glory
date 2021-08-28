#pragma once
#include "GraphicsModule.h"

namespace Glory
{
	class FrameStates
	{
	public:
		FrameStates(GraphicsModule* pModule);
		virtual ~FrameStates();

	protected:
		virtual void Initialize() {}
		virtual void OnFrameStart() {}
		virtual void OnFrameEnd() {}

	private:
		friend class GraphicsModule;
		friend class GraphicsThread;
		GraphicsModule* m_pModule;
	};
}
