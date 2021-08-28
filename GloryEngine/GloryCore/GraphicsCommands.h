#pragma once
#include <string>
#include <glm/glm.hpp>
#include <vector>

namespace Glory
{
	struct TestCommand
	{
		TestCommand() {}
		TestCommand(const std::string& text) : Text(text) {}

		std::string Text;
	};

	struct BeginRenderPassCommand
	{
		BeginRenderPassCommand() : Offset({ 0,0 }), Extent({ 0,0 }), Depth(1.0f), Stencil(0) {}
		BeginRenderPassCommand(const glm::u32vec2& offset, const glm::u32vec2& extent) : Offset(offset), Extent(extent), Depth(1.0f), Stencil(0) {}

		glm::u32vec2 Offset;
		glm::u32vec2 Extent;
		std::vector<glm::vec4> ClearValues;
		float Depth;
		uint32_t Stencil;
	};

	struct EndRenderPassCommand
	{
		EndRenderPassCommand() {}
	};

	struct BindPipelineCommand
	{
		BindPipelineCommand() {}
	};
}


//auto deviceData = m_pDeviceManager->GetSelectedDevice()->GetLogicalDeviceData();
//vk::CommandPool commandPool = m_pDeviceManager->GetSelectedDevice()->GetGraphicsCommandPool();
//
//// Create command buffers
//m_CommandBuffers.resize(m_pSwapChain->GetImageCount());
//
//vk::CommandBufferAllocateInfo commandBufferAllocateInfo = vk::CommandBufferAllocateInfo()
//.setCommandPool(commandPool)
//.setLevel(vk::CommandBufferLevel::ePrimary)
//.setCommandBufferCount((uint32_t)m_CommandBuffers.size());
//
//if (deviceData.LogicalDevice.allocateCommandBuffers(&commandBufferAllocateInfo, m_CommandBuffers.data()) != vk::Result::eSuccess)
//throw std::runtime_error("failed to allocate command buffers!");
//
//auto swapchainExtent = m_pSwapChain->GetExtent();
//
//for (size_t i = 0; i < m_CommandBuffers.size(); i++)
//{
//    // Start the command buffer
//    vk::CommandBufferBeginInfo commandBufferBeginInfo = vk::CommandBufferBeginInfo()
//        .setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse)
//        .setPInheritanceInfo(nullptr);
//
//    if (m_CommandBuffers[i].begin(&commandBufferBeginInfo) != vk::Result::eSuccess)
//        throw std::runtime_error("failed to begin recording command buffer!");
//
//    // Start a render pass
//    vk::Rect2D renderArea = vk::Rect2D()
//        .setOffset({ 0,0 })
//        .setExtent(swapchainExtent);
//
//    vk::ClearColorValue clearColorValue = vk::ClearColorValue()
//        .setFloat32({ 0.0f, 0.0f, 0.0f, 1.0f });
//
//    vk::ClearColorValue clearPosValue = vk::ClearColorValue()
//        .setFloat32({ 0.0f, 0.0f, 0.0f, 1.0f });
//
//    vk::ClearColorValue clearNormalValue = vk::ClearColorValue()
//        .setFloat32({ 0.737f, 0.737f, 1.0f, 1.0f });
//
//    std::array<vk::ClearValue, 4> clearColors{};
//    clearColors[0].setColor(clearColorValue);
//    clearColors[1].setColor(clearPosValue);
//    clearColors[2].setColor(clearNormalValue);
//    clearColors[3].setDepthStencil({ 1.0f, 0 });
//
//    vk::RenderPassBeginInfo renderPassBeginInfo = vk::RenderPassBeginInfo()
//        .setRenderPass(m_pRenderPass->m_RenderPass)
//        .setFramebuffer(m_pRenderPass->m_Framebuffers[i])
//        .setRenderArea(renderArea)
//        .setClearValueCount(static_cast<uint32_t>(clearColors.size()))
//        .setPClearValues(clearColors.data());
//
//    m_CommandBuffers[i].beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);
//    m_CommandBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, m_pRenderPipeline->m_GraphicsPipeline);
//
//    vk::Buffer vertexBuffers[] = { m_pVertexBuffer->GetBuffer() };
//    vk::DeviceSize offsets[] = { 0 };
//    m_CommandBuffers[i].bindVertexBuffers(0, 1, vertexBuffers, offsets);
//    m_CommandBuffers[i].bindIndexBuffer(m_pIndexBuffer->GetBuffer(), 0, vk::IndexType::eUint32);
//
//    m_CommandBuffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pRenderPipeline->m_PipelineLayout, 0, 1, &m_pRenderPipeline->m_DescriptorSets[i], 0, nullptr);
//
//    m_CommandBuffers[i].drawIndexed(static_cast<uint32_t>(m_pMesh->GetIndexCount()), 1, 0, 0, 0);
//    m_CommandBuffers[i].endRenderPass();
//    m_CommandBuffers[i].end();
//}