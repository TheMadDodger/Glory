#include "RenderPassCommandHandlers.h"
#include <vulkan/vulkan.hpp>
#include "VulkanRenderPass.h"

namespace Glory
{
	void BeginRenderPassCommandHandler::OnInvoke(BeginRenderPassCommand commandData)
	{
		// Start a render pass
        vk::Rect2D renderArea = vk::Rect2D()
            .setOffset(vk::Offset2D(commandData.Offset.x, commandData.Offset.y))
            .setExtent(vk::Extent2D(commandData.Extent.x, commandData.Extent.y));

        //std::vector<vk::ClearColorValue> clearColors = std::vector<vk::ClearColorValue>(commandData.ClearValues.size());
        std::vector<vk::ClearValue> clearColors = std::vector<vk::ClearValue>(commandData.ClearValues.size() + 1);
        for (size_t i = 0; i < clearColors.size() - 1; i++)
        {
            glm::vec4 value = commandData.ClearValues[i];
            vk::ClearColorValue clearColor;
            memcpy(&clearColor, (const void*)&value, sizeof(float) * 4);
            clearColors[i].setColor(clearColor);
        }

        clearColors[commandData.ClearValues.size()].setDepthStencil(vk::ClearDepthStencilValue(commandData.Depth, commandData.Stencil));

        VulkanRenderPass* pRenderPass = m_pModule->GetVulkanRenderPass();

        vk::RenderPassBeginInfo renderPassBeginInfo = vk::RenderPassBeginInfo()
            .setRenderPass(pRenderPass->GetRenderPass())
            .setFramebuffer(pRenderPass->GetCurrentFrameBuffer())
            .setRenderArea(renderArea)
            .setClearValueCount(static_cast<uint32_t>(clearColors.size()))
            .setPClearValues(clearColors.data());

        vk::CommandBuffer commandBuffer = m_pModule->GetVulkanCommandBuffers()->GetCurrentFrameCommandBuffer();

        commandBuffer.beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);
	}

    void EndRenderPassCommandHandler::OnInvoke(EndRenderPassCommand commandData)
    {
        vk::CommandBuffer commandBuffer = m_pModule->GetVulkanCommandBuffers()->GetCurrentFrameCommandBuffer();
        commandBuffer.endRenderPass();
    }
}
