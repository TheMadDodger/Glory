//#pragma once
//#include <GraphicsCommandHandler.h>
//#include "VulkanGraphicsModule.h"
//
//namespace Glory
//{
//	class BeginRenderPassCommandHandler : GraphicsCommandHandler<BeginRenderPassCommand>
//	{
//	public:
//		BeginRenderPassCommandHandler(GraphicsModule* pModule) : m_pModule((VulkanGraphicsModule*)pModule) {}
//		virtual ~BeginRenderPassCommandHandler() {}
//
//	private:
//		virtual void OnInvoke(BeginRenderPassCommand commandData) override;
//
//	private:
//		VulkanGraphicsModule* m_pModule;
//	};
//
//	class EndRenderPassCommandHandler : GraphicsCommandHandler<EndRenderPassCommand>
//	{
//	public:
//		EndRenderPassCommandHandler(GraphicsModule* pModule) : m_pModule((VulkanGraphicsModule*)pModule) {}
//		virtual ~EndRenderPassCommandHandler() {}
//
//	private:
//		virtual void OnInvoke(EndRenderPassCommand commandData) override;
//
//	private:
//		VulkanGraphicsModule* m_pModule;
//	};
//}
