#pragma once
#include <GraphicsCommandHandler.h>
#include "VulkanGraphicsModule.h"

namespace Glory
{
	class BindPipelineCommandHandler : GraphicsCommandHandler<BindPipelineCommand>
	{
	public:
		BindPipelineCommandHandler(GraphicsModule* pModule) : m_pModule((VulkanGraphicsModule*)pModule) {}
		virtual ~BindPipelineCommandHandler() {}

	private:
		virtual void OnInvoke(BindPipelineCommand commandData) override;

	private:
		VulkanGraphicsModule* m_pModule;
	};
}
