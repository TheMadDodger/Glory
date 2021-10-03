//#include "PipelineCommandHandlers.h"
//
//namespace Glory
//{
//	void BindPipelineCommandHandler::OnInvoke(BindPipelineCommand command)
//	{
//		vk::CommandBuffer commandBuffer = m_pModule->GetVulkanCommandBuffers()->GetCurrentFrameCommandBuffer();
//		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pModule->GetVulkanGraphicsPipeline()->GetPipeline());
//	}
//}