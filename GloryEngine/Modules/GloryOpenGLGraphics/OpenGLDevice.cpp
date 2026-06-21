#include "OpenGLDevice.h"
#include "OpenGLData.h"
#include "OpenGLCommandImpl.h"
#include "OpenGLGraphicsModule.h"

#include <IEngine.h>
#include <Debug.h>
#include <Window.h>

#include <PipelineData.h>
#include <MeshData.h>
#include <ImageData.h>
#include <TextureData.h>
#include <CubemapData.h>
#include <FileData.h>
#include <GloryAssert.h>

namespace Glory
{
	GL_CommandBuffer::GL_CommandBuffer(size_t capacity) :
		m_CommandsCapacity(capacity), m_Commands(new GL_CommandData[capacity]),
		m_GLCurrentPrimitives(PrimitiveTypes.at(PrimitiveType::Triangles))
	{
	}

	OpenGLDevice::OpenGLDevice(OpenGLGraphicsModule* pModule): GraphicsDevice(pModule)
	{
		m_APIFeatures = APIFeatures::All & ~APIFeatures::PushConstants;
	}

	OpenGLDevice::~OpenGLDevice()
	{
		m_Swapchains.FreeAll(std::bind(&OpenGLDevice::FreeSwapchain, this, std::placeholders::_1));
		m_Pipelines.FreeAll(std::bind(&OpenGLDevice::FreePipeline, this, std::placeholders::_1));
		m_Shaders.FreeAll(std::bind(&OpenGLDevice::FreeShader, this, std::placeholders::_1));
		m_RenderPasses.FreeAll(std::bind(&OpenGLDevice::FreeRenderPass, this, std::placeholders::_1));
		m_RenderTextures.FreeAll(std::bind(&OpenGLDevice::FreeRenderTexture, this, std::placeholders::_1));
		m_Textures.FreeAll(std::bind(&OpenGLDevice::FreeTexture, this, std::placeholders::_1));
		m_Meshes.FreeAll(std::bind(&OpenGLDevice::FreeMesh, this, std::placeholders::_1));
		m_Buffers.FreeAll(std::bind(&OpenGLDevice::FreeBuffer, this, std::placeholders::_1));
		m_Sets.FreeAll(std::bind(&OpenGLDevice::FreeDescriptorSet, this, std::placeholders::_1));
		m_SetLayouts.FreeAll(std::bind(&OpenGLDevice::FreeDescriptorSetLayout, this, std::placeholders::_1));
	}

	OpenGLGraphicsModule* OpenGLDevice::GraphicsModule()
	{
		return static_cast<OpenGLGraphicsModule*>(m_pModule);
	}

	void OpenGLDevice::SetCommandBufferEmulationEnabled(bool enable)
	{
		m_IsCommandBufferEmulationEnabled = enable;
		if (enable)
			Debug().LogInfo("OpenGLDevice command buffer emulation is enabled!");
		else
			Debug().LogInfo("OpenGLDevice command buffer emulation is disabled!");
	}

	uint32_t OpenGLDevice::GetGLTextureID(TextureHandle texture)
	{
		GL_Texture* glTexture = m_Textures.Find(texture);
		if (!glTexture)
		{
			Debug().LogError("OpenGLDevice::GetGLTextureID: Invalid texture handle.");
			return 0;
		}
		return glTexture->m_GLTextureID;
	}

#pragma region Commands

	CommandBufferHandle OpenGLDevice::CreateCommandBuffer()
	{
		if (m_FreeCommandBuffers.empty())
		{
			CommandBufferHandle handle;
			m_CommandBuffers.Emplace(handle, GL_CommandBuffer());
			return handle;
		}

		const CommandBufferHandle commandBuffer = m_FreeCommandBuffers.front();
		m_FreeCommandBuffers.pop();
		return commandBuffer;
	}

	void OpenGLDevice::Begin(CommandBufferHandle commandBuffer)
	{
		GL_CommandBuffer* glCommandBuffer = m_CommandBuffers.Find(commandBuffer);
		if (!glCommandBuffer)
		{
			Debug().LogError("OpenGLDevice::Begin: Invalid command buffer handle.");
			return;
		}
		if (m_IsCommandBufferEmulationEnabled && glCommandBuffer->m_CommandsSize > 0)
		{
			Debug().LogError("OpenGLDevice::Begin: Command buffer already recording.");
			return;
		}
		PushCommand(*glCommandBuffer, GLCommandType::Begin);
	}

	void OpenGLDevice::BeginRenderPass(CommandBufferHandle commandBuffer, RenderPassHandle renderPass)
	{
		GL_CommandBuffer* glCommandBuffer = m_CommandBuffers.Find(commandBuffer);
		if (!glCommandBuffer)
		{
			Debug().LogError("OpenGLDevice::BeginRenderPass: Invalid command buffer handle.");
			return;
		}
		if (m_IsCommandBufferEmulationEnabled && glCommandBuffer->m_CommandsSize == 0)
		{
			Debug().LogError("OpenGLDevice::BeginRenderPass: Command buffer has not started recording yet.");
			return;
		}

		GL_CommandData command = GLCommandType::BeginRenderPass;
		command.m_RenderPass = renderPass;
		PushCommand(*glCommandBuffer, std::move(command));
	}

	void OpenGLDevice::BeginPipeline(CommandBufferHandle commandBuffer, PipelineHandle pipeline)
	{
		GL_CommandBuffer* glCommandBuffer = m_CommandBuffers.Find(commandBuffer);
		if (!glCommandBuffer)
		{
			Debug().LogError("OpenGLDevice::BeginPipeline: Invalid command buffer handle.");
			return;
		}
		if (m_IsCommandBufferEmulationEnabled && glCommandBuffer->m_CommandsSize == 0)
		{
			Debug().LogError("OpenGLDevice::BeginPipeline: Command buffer has not started recording yet.");
			return;
		}

		GL_CommandData command = GLCommandType::BeginPipeline;
		command.m_Pipeline = pipeline;
		PushCommand(*glCommandBuffer, std::move(command));
	}

	void OpenGLDevice::End(CommandBufferHandle commandBuffer)
	{
		GL_CommandBuffer* glCommandBuffer = m_CommandBuffers.Find(commandBuffer);
		if (!glCommandBuffer)
		{
			Debug().LogError("OpenGLDevice::End: Invalid command buffer handle.");
			return;
		}
		if (m_IsCommandBufferEmulationEnabled && glCommandBuffer->m_CommandsSize == 0)
		{
			Debug().LogError("OpenGLDevice::End: Command buffer has not started recording yet.");
			return;
		}
		PushCommand(*glCommandBuffer, GLCommandType::End);
	}

	void OpenGLDevice::EndRenderPass(CommandBufferHandle commandBuffer)
	{
		GL_CommandBuffer* glCommandBuffer = m_CommandBuffers.Find(commandBuffer);
		if (!glCommandBuffer)
		{
			Debug().LogError("OpenGLDevice::EndRenderPass: Invalid command buffer handle.");
			return;
		}
		if (m_IsCommandBufferEmulationEnabled && glCommandBuffer->m_CommandsSize == 0)
		{
			Debug().LogError("OpenGLDevice::EndRenderPass: Command buffer has not started recording yet.");
			return;
		}

		PushCommand(*glCommandBuffer, GLCommandType::EndRenderPass);
	}

	void OpenGLDevice::EndPipeline(CommandBufferHandle commandBuffer)
	{
		GL_CommandBuffer* glCommandBuffer = m_CommandBuffers.Find(commandBuffer);
		if (!glCommandBuffer)
		{
			Debug().LogError("OpenGLDevice::EndPipeline: Invalid command buffer handle.");
			return;
		}
		if (m_IsCommandBufferEmulationEnabled && glCommandBuffer->m_CommandsSize == 0)
		{
			Debug().LogError("OpenGLDevice::EndPipeline: Command buffer has not started recording yet.");
			return;
		}

		PushCommand(*glCommandBuffer, GLCommandType::EndPipeline);
	}

	void OpenGLDevice::BindDescriptorSets(CommandBufferHandle commandBuffer, PipelineHandle pipeline, const std::vector<DescriptorSetHandle>& sets, uint32_t)
	{
		GL_CommandBuffer* glCommandBuffer = m_CommandBuffers.Find(commandBuffer);
		if (!glCommandBuffer)
		{
			Debug().LogError("OpenGLDevice::BindDescriptorSets: Invalid command buffer handle.");
			return;
		}
		if (m_IsCommandBufferEmulationEnabled && glCommandBuffer->m_CommandsSize == 0)
		{
			Debug().LogError("OpenGLDevice::BindDescriptorSets: Command buffer has not started recording yet.");
			return;
		}

		/* Push command for each set so we can stay cache friendly */
		for (size_t i = 0; i < sets.size(); ++i)
		{
			GL_CommandData commandData = GLCommandType::BindDescriptorSets;
			commandData.m_Pipeline = pipeline;
			commandData.m_DescriptorSet = sets[i];
			PushCommand(*glCommandBuffer, std::move(commandData));
		}
	}

	void OpenGLDevice::PushConstants(CommandBufferHandle commandBuffer, PipelineHandle pipeline, uint32_t offset, uint32_t size, const void* data, ShaderTypeFlag)
	{
		if (size > PushConstantsMaxSize)
		{
			Debug().LogError("OpenGLDevice::PushConstants: Push constant data size exceeds maximum.");
			return;
		}

		GL_CommandBuffer* glCommandBuffer = m_CommandBuffers.Find(commandBuffer);
		if (!glCommandBuffer)
		{
			Debug().LogError("OpenGLDevice::PushConstants: Invalid command buffer handle.");
			return;
		}
		if (m_IsCommandBufferEmulationEnabled && glCommandBuffer->m_CommandsSize == 0)
		{
			Debug().LogError("OpenGLDevice::PushConstants: Command buffer has not started recording yet.");
			return;
		}

		GL_CommandData commandData = GLCommandType::PushConstants;
		commandData.m_Pipeline = pipeline;
		commandData.m_PushConstantsOffset = offset;
		commandData.m_PushConstantsSize = size;
		commandData.m_PushConstantsDataIndex = glCommandBuffer->m_PushConstantData.size();
		auto& array = glCommandBuffer->m_PushConstantData.emplace_back();
		std::memcpy(array.data(), data, size);
		PushCommand(*glCommandBuffer, std::move(commandData));
	}

	void OpenGLDevice::DrawMesh(CommandBufferHandle commandBuffer, MeshHandle handle)
	{
		GL_CommandBuffer* glCommandBuffer = m_CommandBuffers.Find(commandBuffer);
		if (!glCommandBuffer)
		{
			Debug().LogError("OpenGLDevice::DrawMesh: Invalid command buffer handle.");
			return;
		}
		if (m_IsCommandBufferEmulationEnabled && glCommandBuffer->m_CommandsSize == 0)
		{
			Debug().LogError("OpenGLDevice::DrawMesh: Command buffer has not started recording yet.");
			return;
		}

		GL_CommandData commandData = GLCommandType::DrawMesh;
		commandData.m_Mesh = handle;
		PushCommand(*glCommandBuffer, std::move(commandData));
	}

	void OpenGLDevice::Dispatch(CommandBufferHandle commandBuffer, uint32_t x, uint32_t y, uint32_t z)
	{
		GL_CommandBuffer* glCommandBuffer = m_CommandBuffers.Find(commandBuffer);
		if (!glCommandBuffer)
		{
			Debug().LogError("OpenGLDevice::Dispatch: Invalid command buffer handle.");
			return;
		}
		if (m_IsCommandBufferEmulationEnabled && glCommandBuffer->m_CommandsSize == 0)
		{
			Debug().LogError("OpenGLDevice::Dispatch: Command buffer has not started recording yet.");
			return;
		}

		GL_CommandData commandData = GLCommandType::Dispatch;
		commandData.m_XYZ = { x, y, z, 0u };
		PushCommand(*glCommandBuffer, std::move(commandData));
	}

	void OpenGLDevice::SetStencilTestEnabled(CommandBufferHandle commandBuffer, bool enable)
	{
		GL_CommandBuffer* glCommandBuffer = m_CommandBuffers.Find(commandBuffer);
		if (!glCommandBuffer)
		{
			Debug().LogError("OpenGLDevice::SetStencilTestEnabled: Invalid command buffer handle.");
			return;
		}
		if (m_IsCommandBufferEmulationEnabled && glCommandBuffer->m_CommandsSize == 0)
		{
			Debug().LogError("OpenGLDevice::SetStencilTestEnabled: Command buffer has not started recording yet.");
			return;
		}

		GL_CommandData commandData = GLCommandType::SetStencilTestEnabled;
		commandData.m_Enable = enable ? 1 : 0;
		PushCommand(*glCommandBuffer, std::move(commandData));
	}

	void OpenGLDevice::SetStencilOp(CommandBufferHandle commandBuffer, CompareOp compareOp,
		Func fail, Func depthFail, Func pass, int8_t reference, uint8_t compareMask)
	{
		GL_CommandBuffer* glCommandBuffer = m_CommandBuffers.Find(commandBuffer);
		if (!glCommandBuffer)
		{
			Debug().LogError("OpenGLDevice::SetStencilOp: Invalid command buffer handle.");
			return;
		}
		if (m_IsCommandBufferEmulationEnabled && glCommandBuffer->m_CommandsSize == 0)
		{
			Debug().LogError("OpenGLDevice::SetStencilOp: Command buffer has not started recording yet.");
			return;
		}

		GL_CommandData commandData = GLCommandType::SetStencilOp;
		commandData.m_CompareOp = uint8_t(compareOp);
		commandData.m_Fail = uint8_t(fail);
		commandData.m_DepthFail = uint8_t(depthFail);
		commandData.m_Pass = uint8_t(pass);
		commandData.m_Reference = reference;
		commandData.m_Mask = compareMask;
		PushCommand(*glCommandBuffer, std::move(commandData));
	}

	void OpenGLDevice::SetStencilWriteMask(CommandBufferHandle commandBuffer, uint8_t mask)
	{
		GL_CommandBuffer* glCommandBuffer = m_CommandBuffers.Find(commandBuffer);
		if (!glCommandBuffer)
		{
			Debug().LogError("OpenGLDevice::SetStencilWriteMask: Invalid command buffer handle.");
			return;
		}
		if (m_IsCommandBufferEmulationEnabled && glCommandBuffer->m_CommandsSize == 0)
		{
			Debug().LogError("OpenGLDevice::SetStencilWriteMask: Command buffer has not started recording yet.");
			return;
		}

		GL_CommandData commandData = GLCommandType::SetStencilWriteMask;
		commandData.m_Mask = mask;
		PushCommand(*glCommandBuffer, std::move(commandData));
	}

	void OpenGLDevice::Commit(CommandBufferHandle commandBuffer, const std::vector<SemaphoreHandle>&, const std::vector<SemaphoreHandle>&)
	{
		GL_CommandBuffer* glCommandBuffer = m_CommandBuffers.Find(commandBuffer);
		if (!glCommandBuffer)
		{
			Debug().LogError("OpenGLDevice::Commit: Invalid command buffer handle.");
			return;
		}
		if (m_IsCommandBufferEmulationEnabled && glCommandBuffer->m_CommandsSize == 0)
		{
			Debug().LogError("OpenGLDevice::Commit: Command buffer has no commands recorded.");
			return;
		}

		if (m_IsCommandBufferEmulationEnabled && glCommandBuffer->m_Commands[glCommandBuffer->m_CommandsSize - 1].m_CommandType != GLCommandType::End)
		{
			Debug().LogError("OpenGLDevice::Commit: Command buffer has not finished recording.");
			return;
		}

		if (m_IsCommandBufferEmulationEnabled)
			OpenGLCommandImpl::Commit_Impl(*this, *glCommandBuffer);

		glCommandBuffer->m_Fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glFlush();
	}

	GraphicsDevice::WaitResult OpenGLDevice::Wait(CommandBufferHandle commandBuffer, uint64_t timeout)
	{
		GL_CommandBuffer* glCommandBuffer = m_CommandBuffers.Find(commandBuffer);
		if (!glCommandBuffer)
		{
			Debug().LogError("OpenGLDevice::Wait: Invalid command buffer handle.");
			return WaitResult::WR_Fail;
		}

		if (!glCommandBuffer->m_Fence) return WaitResult::WR_Success;
		const GLenum result = glClientWaitSync(glCommandBuffer->m_Fence, 0, timeout);
		switch (result)
		{
		case GL_ALREADY_SIGNALED:
		case GL_CONDITION_SATISFIED:
			glDeleteSync(glCommandBuffer->m_Fence);
			glCommandBuffer->m_Fence = nullptr;
			return WaitResult::WR_Success;
		case GL_TIMEOUT_EXPIRED:
			return WaitResult::WR_Timeout;
		case GL_WAIT_FAILED:
			return WaitResult::WR_Fail;
		}

		GLORY_ASSERT_UNREACHABLE_CODE();
	}

	void OpenGLDevice::Release(CommandBufferHandle commandBuffer)
	{
		const GL_CommandBuffer* glCommandBuffer = m_CommandBuffers.Find(commandBuffer);
		if (!glCommandBuffer)
		{
			Debug().LogError("OpenGLDevice::Release: Invalid command buffer handle.");
			return;
		}
		if (glCommandBuffer->m_CommandsSize > 0)
			Reset(commandBuffer);

		m_FreeCommandBuffers.push(commandBuffer);
	}

	void OpenGLDevice::Reset(CommandBufferHandle commandBuffer)
	{
		GL_CommandBuffer* glCommandBuffer = m_CommandBuffers.Find(commandBuffer);
		if (!glCommandBuffer)
		{
			Debug().LogError("OpenGLDevice::Reset: Invalid command buffer handle.");
			return;
		}
		if (m_IsCommandBufferEmulationEnabled && glCommandBuffer->m_CommandsSize == 0)
		{
			Debug().LogError("OpenGLDevice::Reset: Command buffer has not started recording yet.");
			return;
		}

		if (glCommandBuffer->m_Fence)
		{
			Debug().LogError("OpenGLDevice::Reset: Command buffer is currently being executed.");
			return;
		}

		glCommandBuffer->m_CommandsSize = 0;
		glCommandBuffer->m_PushConstantData.clear();
		glCommandBuffer->m_Fence = nullptr;
	}

	void OpenGLDevice::SetViewport(CommandBufferHandle commandBuffer, float x, float y, float width, float height, float, float)
	{
		GL_CommandBuffer* glCommandBuffer = m_CommandBuffers.Find(commandBuffer);
		if (!glCommandBuffer)
		{
			Debug().LogError("OpenGLDevice::SetViewport: Invalid command buffer handle.");
			return;
		}
		if (m_IsCommandBufferEmulationEnabled && glCommandBuffer->m_CommandsSize == 0)
		{
			Debug().LogError("OpenGLDevice::SetViewport: Command buffer has not started recording yet.");
			return;
		}

		GL_CommandData commandData = GLCommandType::SetViewport;
		commandData.m_XYZFloat = { x, y, width, height };
		PushCommand(*glCommandBuffer, std::move(commandData));
	}

	void OpenGLDevice::SetScissor(CommandBufferHandle commandBuffer, int x, int y, uint32_t width, uint32_t height)
	{
		GL_CommandBuffer* glCommandBuffer = m_CommandBuffers.Find(commandBuffer);
		if (!glCommandBuffer)
		{
			Debug().LogError("OpenGLDevice::SetScissor: Invalid command buffer handle.");
			return;
		}
		if (m_IsCommandBufferEmulationEnabled && glCommandBuffer->m_CommandsSize == 0)
		{
			Debug().LogError("OpenGLDevice::SetScissor: Command buffer has not started recording yet.");
			return;
		}

		GL_CommandData commandData = GLCommandType::SetScissor;
		commandData.m_XYZSigned = { x, y, width, height };
		PushCommand(*glCommandBuffer, std::move(commandData));
	}

	void OpenGLDevice::PipelineBarrier(CommandBufferHandle commandBuffer, const std::vector<BufferBarrier>& buffers,
		const std::vector<ImageBarrier>& images, PipelineStageFlagBits, PipelineStageFlagBits)
	{
		GL_CommandBuffer* glCommandBuffer = m_CommandBuffers.Find(commandBuffer);
		if (!glCommandBuffer)
		{
			Debug().LogError("OpenGLDevice::Reset: Invalid command buffer handle.");
			return;
		}
		if (m_IsCommandBufferEmulationEnabled && glCommandBuffer->m_CommandsSize == 0)
		{
			Debug().LogError("OpenGLDevice::PipelineBarrier: Command buffer has not started recording yet.");
			return;
		}

		GLbitfield barrierBitField = 0;

		for (size_t i = 0; i < buffers.size(); ++i)
		{
			GL_Buffer* glBuffer = m_Buffers.Find(buffers[i].m_Buffer);
			switch (glBuffer->m_GLTarget)
			{
			case GL_UNIFORM_BUFFER:
				barrierBitField |= GL_UNIFORM_BARRIER_BIT;
				break;
			case GL_SHADER_STORAGE_BUFFER:
				barrierBitField |= GL_SHADER_STORAGE_BARRIER_BIT;
				break;
			default:
				break;
			}
		}

		if (!images.empty())
		{
			barrierBitField |= GL_SHADER_IMAGE_ACCESS_BARRIER_BIT;
			barrierBitField |= GL_FRAMEBUFFER_BARRIER_BIT;
		}

		if (barrierBitField == 0) return;
		GL_CommandData commandData = GLCommandType::PipelineBarrier;
		commandData.m_FlagBits = barrierBitField;
		PushCommand(*glCommandBuffer, std::move(commandData));
	}

	void OpenGLDevice::CopyImage(CommandBufferHandle commandBuffer, TextureHandle src, TextureHandle dst)
	{
		GL_CommandBuffer* glCommandBuffer = m_CommandBuffers.Find(commandBuffer);
		if (!glCommandBuffer)
		{
			Debug().LogError("OpenGLDevice::CopyImage: Invalid command buffer handle.");
			return;
		}
		if (m_IsCommandBufferEmulationEnabled && glCommandBuffer->m_CommandsSize == 0)
		{
			Debug().LogError("OpenGLDevice::CopyImage: Command buffer has not started recording yet.");
			return;
		}

		GL_CommandData commandData = GLCommandType::CopyImage;
		commandData.m_SrcTexture = src;
		commandData.m_DstTexture = dst;
		PushCommand(*glCommandBuffer, std::move(commandData));
	}

	void OpenGLDevice::CopyImageToBuffer(CommandBufferHandle commandBuffer, TextureHandle src, BufferHandle dst)
	{
		GL_CommandBuffer* glCommandBuffer = m_CommandBuffers.Find(commandBuffer);
		if (!glCommandBuffer)
		{
			Debug().LogError("OpenGLDevice::CopyImageToBuffer: Invalid command buffer handle.");
			return;
		}
		if (m_IsCommandBufferEmulationEnabled && glCommandBuffer->m_CommandsSize == 0)
		{
			Debug().LogError("OpenGLDevice::CopyImageToBuffer: Command buffer has not started recording yet.");
			return;
		}

		GL_CommandData commandData = GLCommandType::CopyImageToBuffer;
		commandData.m_SrcTexture = src;
		commandData.m_DstBuffer = dst;
		PushCommand(*glCommandBuffer, std::move(commandData));
	}

	GraphicsDevice::SwapchainResult OpenGLDevice::AcquireNextSwapchainImage(SwapchainHandle swapchain, uint32_t* imageIndex, SemaphoreHandle)
	{
		GL_Swapchain* glSwapchain = m_Swapchains.Find(swapchain);
		if (!glSwapchain)
		{
			Debug().LogError("OpenGLDevice::AqcuireNextSwapchainImage: Invalid swap chain handle.");
			return GraphicsDevice::SwapchainResult::S_Error;
		}

		/* Get next available image */
		*imageIndex = glSwapchain->m_CurrentImageIndex;
		glSwapchain->m_CurrentImageIndex = (glSwapchain->m_CurrentImageIndex + 1) % glSwapchain->m_SwapchainImages.size();
		return GraphicsDevice::SwapchainResult::S_Success;
	}

	GraphicsDevice::SwapchainResult OpenGLDevice::Present(SwapchainHandle swapchain, uint32_t imageIndex, const std::vector<SemaphoreHandle>&)
	{
		GL_Swapchain* glSwapchain = m_Swapchains.Find(swapchain);
		if (!glSwapchain)
		{
			Debug().LogError("OpenGLDevice::Present: Invalid swap chain handle.");
			return GraphicsDevice::SwapchainResult::S_Error;
		}

		/* Blit current image to window back buffer */
		glSwapchain->m_pWindow->MakeGLContextCurrent();
		int width, height;
		glSwapchain->m_pWindow->GetDrawableSize(&width, &height);
		GL_RenderTexture* glRenderTexture = m_RenderTextures.Find(glSwapchain->m_SwapchainImages[imageIndex]);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, glRenderTexture->m_GLFramebufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBlitFramebuffer(0, 0, glRenderTexture->m_Info.Width, glRenderTexture->m_Info.Height,
			0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
		OpenGLGraphicsModule::LogGLError(glGetError());

		/* Swap window */
		glSwapchain->m_pWindow->GLSwapWindow();
		return GraphicsDevice::SwapchainResult::S_Success;
	}

	void OpenGLDevice::WaitIdle()
	{
		glFinish();
	}

#pragma endregion

#pragma region Resource Management

	uint32_t GetBufferUsage(BufferFlags flags)
	{
		if (flags == BF_None)
			return GL_STATIC_DRAW;
		if (flags == BF_Write)
			return GL_DYNAMIC_DRAW;
		if (flags == BF_Read)
			return GL_STATIC_READ;
		if (flags == BF_ReadAndWrite)
			return GL_DYNAMIC_READ;
	}

	BufferHandle OpenGLDevice::CreateBuffer(size_t bufferSize, BufferType type, BufferFlags flags)
	{
		BufferHandle handle;
		GL_Buffer& buffer = m_Buffers.Emplace(handle, GL_Buffer());
		buffer.m_Size = bufferSize;

		glGenBuffers(1, &buffer.m_GLBufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());

		switch (type)
		{
		case Glory::BT_TransferRead:
			buffer.m_GLTarget = GL_COPY_READ_BUFFER;
			buffer.m_GLUsage = GL_DYNAMIC_COPY;
			break;
		case Glory::BT_TransferWrite:
			buffer.m_GLTarget = GL_COPY_WRITE_BUFFER;
			buffer.m_GLUsage = GL_DYNAMIC_COPY;
			break;
		case Glory::BT_Vertex:
			buffer.m_GLTarget = GL_ARRAY_BUFFER;
			buffer.m_GLUsage = GL_STATIC_DRAW;
			break;
		case Glory::BT_Index:
			buffer.m_GLTarget = GL_ELEMENT_ARRAY_BUFFER;
			buffer.m_GLUsage = GL_STATIC_DRAW;
			break;
		case Glory::BT_Storage:
			buffer.m_GLTarget = GL_SHADER_STORAGE_BUFFER;
			buffer.m_GLUsage = GL_STATIC_DRAW;
			break;
		case Glory::BT_Uniform:
			buffer.m_GLTarget = GL_UNIFORM_BUFFER;
			buffer.m_GLUsage = GL_DYNAMIC_DRAW;
			break;
		default:
			break;
		}

		if (flags != BF_None && flags != BF_CopyDst)
			buffer.m_GLUsage = GetBufferUsage(flags);

		glBindBuffer(buffer.m_GLTarget, buffer.m_GLBufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBufferData(buffer.m_GLTarget, buffer.m_Size, NULL, buffer.m_GLUsage);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindBuffer(buffer.m_GLTarget, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());

		return handle;
	}

	void OpenGLDevice::ResizeBuffer(BufferHandle buffer, size_t bufferSize)
	{
		GL_Buffer* glBuffer = m_Buffers.Find(buffer);
		if (!glBuffer)
		{
			Debug().LogError("OpenGLDevice::ResizeBuffer: Invalid buffer handle.");
			return;
		}

		glBuffer->m_Size = bufferSize;
		glBindBuffer(glBuffer->m_GLTarget, glBuffer->m_GLBufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBufferData(glBuffer->m_GLTarget, glBuffer->m_Size, NULL, glBuffer->m_GLUsage);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindBuffer(glBuffer->m_GLTarget, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	size_t OpenGLDevice::BufferSize(BufferHandle buffer)
	{
		GL_Buffer* glBuffer = m_Buffers.Find(buffer);
		if (!glBuffer)
		{
			Debug().LogError("OpenGLDevice::BufferSize: Invalid buffer handle.");
			return 0;
		}
		return glBuffer->m_Size;
	}

	void OpenGLDevice::AssignBuffer(BufferHandle handle, const void* data)
	{
		GL_Buffer* buffer = m_Buffers.Find(handle);
		if (!buffer)
		{
			Debug().LogError("OpenGLDevice::AssignBuffer: Invalid buffer handle.");
			return;
		}

		glBindBuffer(buffer->m_GLTarget, buffer->m_GLBufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBufferData(buffer->m_GLTarget, buffer->m_Size, data, buffer->m_GLUsage);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindBuffer(buffer->m_GLTarget, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void OpenGLDevice::AssignBuffer(BufferHandle handle, const void* data, uint32_t size)
	{
		GL_Buffer* buffer = m_Buffers.Find(handle);
		if (!buffer)
		{
			Debug().LogError("OpenGLDevice::AssignBuffer: Invalid buffer handle");
			return;
		}

		glBindBuffer(buffer->m_GLTarget, buffer->m_GLBufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		if (size > buffer->m_Size)
		{
			buffer->m_Size = size;
			glBufferData(buffer->m_GLTarget, buffer->m_Size, data, buffer->m_GLUsage);
		}
		else
			glBufferSubData(buffer->m_GLTarget, 0, size, data);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindBuffer(buffer->m_GLTarget, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void OpenGLDevice::AssignBuffer(BufferHandle handle, const void* data, uint32_t offset, uint32_t size)
	{
		GL_Buffer* buffer = m_Buffers.Find(handle);
		if (!buffer)
		{
			Debug().LogError("OpenGLDevice::AssignBuffer: Invalid buffer handle");
			return;
		}

		if (offset + size > buffer->m_Size)
		{
			Debug().LogError("OpenGLDevice::AssignBuffer: Attempting to write beyond buffer size");
			return;
		}

		glBindBuffer(buffer->m_GLTarget, buffer->m_GLBufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBufferSubData(buffer->m_GLTarget, offset, size, data);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindBuffer(buffer->m_GLTarget, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void OpenGLDevice::ReadBuffer(BufferHandle handle, void* outData, uint32_t offset, uint32_t size)
	{
		GL_Buffer* buffer = m_Buffers.Find(handle);
		if (!buffer)
		{
			Debug().LogError("OpenGLDevice::ReadBuffer: Invalid buffer handle");
			return;
		}

		if (offset + size > buffer->m_Size)
		{
			Debug().LogError("OpenGLDevice::ReadBuffer: Attempting to read beyond buffer size");
			return;
		}

		glBindBuffer(buffer->m_GLTarget, buffer->m_GLBufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glGetBufferSubData(buffer->m_GLTarget, offset, size, outData);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindBuffer(buffer->m_GLTarget, NULL);
	}

	MeshHandle OpenGLDevice::CreateMesh(std::vector<BufferHandle>&& buffers, uint32_t vertexCount,
		uint32_t indexCount, uint32_t stride, const std::vector<AttributeType>& attributeTypes)
	{
		MeshHandle handle;
		GL_Mesh& mesh = m_Meshes.Emplace(handle, GL_Mesh());
		mesh.m_Buffers = std::move(buffers);
		mesh.m_VertexCount = vertexCount;
		mesh.m_IndexCount = indexCount;

		glGenVertexArrays(1, &mesh.m_GLVertexArrayID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindVertexArray(mesh.m_GLVertexArrayID);
		OpenGLGraphicsModule::LogGLError(glGetError());

		std::vector<GL_Buffer*> glBuffers;

		for (auto& bufferHandle : mesh.m_Buffers)
		{
			GL_Buffer* buffer = m_Buffers.Find(bufferHandle);
			if (!buffer)
			{
				Debug().LogError("OpenGLDevice::CreateMesh: Invalid buffer handle");
				glBindVertexArray(NULL);
				OpenGLGraphicsModule::LogGLError(glGetError());
				glDeleteVertexArrays(1, &mesh.m_GLVertexArrayID);
				OpenGLGraphicsModule::LogGLError(glGetError());
				m_Meshes.Erase(handle);
				return NULL;
			}
			glBindBuffer(buffer->m_GLTarget, buffer->m_GLBufferID);
			OpenGLGraphicsModule::LogGLError(glGetError());
			glBuffers.emplace_back(buffer);
		}

		size_t offset = 0;
		for (uint32_t i = 0; i < attributeTypes.size(); i++)
		{
			switch (attributeTypes[i])
			{
			case Glory::AttributeType::Float:
				glVertexAttribPointer(i, 1, GL_FLOAT, GL_FALSE, stride, (void*)offset);
				OpenGLGraphicsModule::LogGLError(glGetError());
				offset += 1*sizeof(GLfloat);
				break;
			case Glory::AttributeType::Float2:
				glVertexAttribPointer(i, 2, GL_FLOAT, GL_FALSE, stride, (void*)offset);
				OpenGLGraphicsModule::LogGLError(glGetError());
				offset += 2*sizeof(GLfloat);
				break;
			case Glory::AttributeType::Float3:
				glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, stride, (void*)offset);
				OpenGLGraphicsModule::LogGLError(glGetError());
				offset += 3*sizeof(GLfloat);
				break;
			case Glory::AttributeType::Float4:
				glVertexAttribPointer(i, 4, GL_FLOAT, GL_FALSE, stride, (void*)offset);
				OpenGLGraphicsModule::LogGLError(glGetError());
				offset += 4*sizeof(GLfloat);
				break;
			case Glory::AttributeType::UINT:
				glVertexAttribPointer(i, 1, GL_UNSIGNED_INT, GL_FALSE, stride, (void*)offset);
				OpenGLGraphicsModule::LogGLError(glGetError());
				offset += 1*sizeof(GLuint);
				break;
			case Glory::AttributeType::UINT2:
				glVertexAttribPointer(i, 2, GL_UNSIGNED_INT, GL_FALSE, stride, (void*)offset);
				OpenGLGraphicsModule::LogGLError(glGetError());
				offset += 2*sizeof(GLuint);
				break;
			case Glory::AttributeType::UINT3:
				glVertexAttribPointer(i, 3, GL_UNSIGNED_INT, GL_FALSE, stride, (void*)offset);
				OpenGLGraphicsModule::LogGLError(glGetError());
				offset += 3*sizeof(GLuint);
				break;
			case Glory::AttributeType::UINT4:
				glVertexAttribPointer(i, 4, GL_UNSIGNED_INT, GL_FALSE, stride, (void*)offset);
				OpenGLGraphicsModule::LogGLError(glGetError());
				offset += 4*sizeof(GLuint);
				break;
			case Glory::AttributeType::SINT:
				glVertexAttribPointer(i, 1, GL_INT, GL_FALSE, stride, (void*)offset);
				OpenGLGraphicsModule::LogGLError(glGetError());
				offset += 1*sizeof(GLint);
				break;
			case Glory::AttributeType::SINT2:
				glVertexAttribPointer(i, 2, GL_INT, GL_FALSE, stride, (void*)offset);
				OpenGLGraphicsModule::LogGLError(glGetError());
				offset += 2*sizeof(GLint);
				break;
			case Glory::AttributeType::SINT3:
				glVertexAttribPointer(i, 3, GL_INT, GL_FALSE, stride, (void*)offset);
				OpenGLGraphicsModule::LogGLError(glGetError());
				offset += 3*sizeof(GLint);
				break;
			case Glory::AttributeType::SINT4:
				glVertexAttribPointer(i, 4, GL_INT, GL_FALSE, stride, (void*)offset);
				OpenGLGraphicsModule::LogGLError(glGetError());
				offset += 4*sizeof(GLint);
				break;
			default:
				break;
			}

			glEnableVertexAttribArray(i);
		}

		glBindVertexArray(NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());

		for (auto buffer : glBuffers)
		{
			glBindBuffer(buffer->m_GLTarget, NULL);
			OpenGLGraphicsModule::LogGLError(glGetError());
		}

		return handle;
	}

	void OpenGLDevice::UpdateMesh(MeshHandle mesh, std::vector<BufferHandle>&& buffers, uint32_t vertexCount, uint32_t indexCount)
	{
		GL_Mesh* glMesh = m_Meshes.Find(mesh);
		if (!glMesh)
		{
			Debug().LogError("OpenGLDevice::UpdateMesh: Invalid mesh handle.");
			return;
		}

		glMesh->m_IndexCount = indexCount;
		glMesh->m_VertexCount = vertexCount;

		if (buffers.empty()) return;
		glMesh->m_Buffers = std::move(buffers);
		glBindVertexArray(glMesh->m_GLVertexArrayID);
		OpenGLGraphicsModule::LogGLError(glGetError());

		for (auto& bufferHandle : glMesh->m_Buffers)
		{
			GL_Buffer* buffer = m_Buffers.Find(bufferHandle);
			if (!buffer)
			{
				Debug().LogError("OpenGLDevice::UpdateMesh: Invalid buffer handle.");
				return;
			}
			glBindBuffer(buffer->m_GLTarget, buffer->m_GLBufferID);
			OpenGLGraphicsModule::LogGLError(glGetError());
		}
	}

	void OpenGLDevice::UpdateMesh(MeshHandle mesh, MeshData* pMeshData)
	{
		GL_Mesh* glMesh = m_Meshes.Find(mesh);
		if (!glMesh)
		{
			Debug().LogError("OpenGLDevice::UpdateMesh: Invalid mesh handle.");
			return;
		}

		glMesh->m_IndexCount = pMeshData->IndexCount();
		glMesh->m_VertexCount = pMeshData->VertexCount();
		AssignBuffer(glMesh->m_Buffers[0], pMeshData->Vertices(), pMeshData->VertexCount()*pMeshData->VertexSize());
		if (glMesh->m_IndexCount > 0)
			AssignBuffer(glMesh->m_Buffers.back(), pMeshData->Indices(), pMeshData->IndexCount()*sizeof(uint32_t));
	}

	TextureHandle OpenGLDevice::CreateTexture(TextureData* pTexture)
	{
		ImageData* pImageData = pTexture->GetImageData(&m_pModule->GetEngine()->GetResources());
		if (!pImageData) return NULL;

		TextureHandle handle;
		GL_Texture& texture = m_Textures.Emplace(handle, GL_Texture());
		texture.m_Width = pImageData->GetWidth();
		texture.m_Height = pImageData->GetHeight();
		
		texture.m_GLTextureType = GL_TEXTURE_2D;

		if (pImageData->GetBytesPerPixel() == 1)
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glGenTextures(1, &texture.m_GLTextureID);
		OpenGLGraphicsModule::LogGLError(glGetError());

		glBindTexture(texture.m_GLTextureType, texture.m_GLTextureID);
		OpenGLGraphicsModule::LogGLError(glGetError());

		texture.m_GLFormat = Formats.at(pImageData->GetFormat());
		texture.m_GLInternalFormat = pImageData->GetFormat() == PixelFormat::PF_Stencil ? GL_STENCIL_INDEX8 :
			Formats.at(pImageData->GetInternalFormat());
		texture.m_GLDataType = Datatypes.at(pImageData->GetDataType());

		SamplerSettings& sampler = pTexture->GetSamplerSettings();
		texture.m_GLMinFilter = GetMinFilter(sampler.MipmapMode, sampler.MinFilter);
		texture.m_GLMagFilter = Filters.at(sampler.MagFilter);
		texture.m_GLTextureWrapS = Texturewraps.at(sampler.AddressModeU);
		texture.m_GLTextureWrapT = Texturewraps.at(sampler.AddressModeV);
		texture.m_GLTextureWrapR = Texturewraps.at(sampler.AddressModeW);

		glTexImage2D(texture.m_GLTextureType, 0, texture.m_GLInternalFormat, (GLsizei)pImageData->GetWidth(), (GLsizei)pImageData->GetHeight(), 0, texture.m_GLFormat, texture.m_GLDataType, pImageData->GetPixels());
		OpenGLGraphicsModule::LogGLError(glGetError());

		if (sampler.MipmapMode != Filter::F_None)
		{
			glGenerateMipmap(texture.m_GLTextureType);
			OpenGLGraphicsModule::LogGLError(glGetError());
		}

		glTexParameteri(texture.m_GLTextureType, GL_TEXTURE_MIN_FILTER, texture.m_GLMinFilter);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameteri(texture.m_GLTextureType, GL_TEXTURE_MAG_FILTER, texture.m_GLMagFilter);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameteri(texture.m_GLTextureType, GL_TEXTURE_WRAP_S, texture.m_GLTextureWrapS);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameteri(texture.m_GLTextureType, GL_TEXTURE_WRAP_T, texture.m_GLTextureWrapT);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameteri(texture.m_GLTextureType, GL_TEXTURE_WRAP_R, texture.m_GLTextureWrapR);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameterf(texture.m_GLTextureType, GL_TEXTURE_MIN_LOD, sampler.MinLOD);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameterf(texture.m_GLTextureType, GL_TEXTURE_MAX_LOD, sampler.MaxLOD);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameterf(texture.m_GLTextureType, GL_TEXTURE_LOD_BIAS, sampler.MipLODBias);
		OpenGLGraphicsModule::LogGLError(glGetError());

		float aniso = 0.0f;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
		OpenGLGraphicsModule::LogGLError(glGetError());
		aniso = std::min(sampler.MaxAnisotropy, aniso);
		glTexParameterf(texture.m_GLTextureType, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
		OpenGLGraphicsModule::LogGLError(glGetError());

		glCreateSamplers(1, &texture.m_GLSamplerID);
		glSamplerParameteri(texture.m_GLSamplerID, GL_TEXTURE_MIN_FILTER, texture.m_GLMinFilter);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glSamplerParameteri(texture.m_GLSamplerID, GL_TEXTURE_MAG_FILTER, texture.m_GLMagFilter);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glSamplerParameteri(texture.m_GLSamplerID, GL_TEXTURE_WRAP_S, texture.m_GLTextureWrapS);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glSamplerParameteri(texture.m_GLSamplerID, GL_TEXTURE_WRAP_T, texture.m_GLTextureWrapT);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glSamplerParameteri(texture.m_GLSamplerID, GL_TEXTURE_WRAP_R, texture.m_GLTextureWrapR);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glSamplerParameterf(texture.m_GLSamplerID, GL_TEXTURE_MIN_LOD, sampler.MinLOD);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glSamplerParameterf(texture.m_GLSamplerID, GL_TEXTURE_MAX_LOD, sampler.MaxLOD);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glSamplerParameterf(texture.m_GLSamplerID, GL_TEXTURE_LOD_BIAS, sampler.MipLODBias);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glSamplerParameterf(texture.m_GLSamplerID, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
		OpenGLGraphicsModule::LogGLError(glGetError());

		texture.m_GLBindlessHandle = glGetTextureSamplerHandleARB(texture.m_GLTextureID, texture.m_GLSamplerID);
		glMakeTextureHandleResidentARB(texture.m_GLBindlessHandle);

		glBindTexture(texture.m_GLTextureType, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());

		return handle;
	}

	TextureHandle OpenGLDevice::CreateTexture(CubemapData* pCubemap)
	{
		ImageData* pImageData = pCubemap->GetImageData(&m_pModule->GetEngine()->GetResources(), 0);
		if (!pImageData) return NULL;

		TextureHandle handle;
		GL_Texture& texture = m_Textures.Emplace(handle, GL_Texture());
		texture.m_Width = pImageData->GetWidth();
		texture.m_Height = pImageData->GetHeight();
		texture.m_GLTextureType = GL_TEXTURE_CUBE_MAP;

		texture.m_GLFormat = Formats.at(pImageData->GetFormat());
		texture.m_GLInternalFormat = pImageData->GetFormat() == PixelFormat::PF_Stencil ? GL_STENCIL_INDEX8 :
			Formats.at(pImageData->GetInternalFormat());
		texture.m_GLDataType = Datatypes.at(pImageData->GetDataType());

		SamplerSettings& sampler = pCubemap->GetSamplerSettings();
		texture.m_GLMinFilter = GetMinFilter(sampler.MipmapMode, sampler.MinFilter);
		texture.m_GLMagFilter = Filters.at(sampler.MagFilter);
		texture.m_GLTextureWrapS = Texturewraps.at(sampler.AddressModeU);
		texture.m_GLTextureWrapT = Texturewraps.at(sampler.AddressModeV);
		texture.m_GLTextureWrapR = Texturewraps.at(sampler.AddressModeW);

		if (pImageData->GetBytesPerPixel() == 1)
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glGenTextures(1, &texture.m_GLTextureID);
		OpenGLGraphicsModule::LogGLError(glGetError());

		glBindTexture(texture.m_GLTextureType, texture.m_GLTextureID);
		OpenGLGraphicsModule::LogGLError(glGetError());

		for (unsigned int i = 0; i < 6; ++i)
		{
			ImageData* pImageData = pCubemap->GetImageData(&m_pModule->GetEngine()->GetResources(), i);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, texture.m_GLInternalFormat,
				(GLsizei)pImageData->GetWidth(), (GLsizei)pImageData->GetHeight(), 0, texture.m_GLFormat, texture.m_GLDataType, pImageData->GetPixels());
			OpenGLGraphicsModule::LogGLError(glGetError());
		}

		glTexParameteri(texture.m_GLTextureType, GL_TEXTURE_MIN_FILTER, texture.m_GLMinFilter);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameteri(texture.m_GLTextureType, GL_TEXTURE_MAG_FILTER, texture.m_GLMagFilter);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameteri(texture.m_GLTextureType, GL_TEXTURE_WRAP_S, texture.m_GLTextureWrapS);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameteri(texture.m_GLTextureType, GL_TEXTURE_WRAP_T, texture.m_GLTextureWrapT);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameteri(texture.m_GLTextureType, GL_TEXTURE_WRAP_R, texture.m_GLTextureWrapR);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameterf(texture.m_GLTextureType, GL_TEXTURE_MIN_LOD, sampler.MinLOD);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameterf(texture.m_GLTextureType, GL_TEXTURE_MAX_LOD, sampler.MaxLOD);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameterf(texture.m_GLTextureType, GL_TEXTURE_LOD_BIAS, sampler.MipLODBias);
		OpenGLGraphicsModule::LogGLError(glGetError());

		float aniso = 0.0f;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
		OpenGLGraphicsModule::LogGLError(glGetError());
		aniso = std::min(sampler.MaxAnisotropy, aniso);
		glTexParameterf(texture.m_GLTextureType, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
		OpenGLGraphicsModule::LogGLError(glGetError());

		if (sampler.MipmapMode != Filter::F_None)
		{
			glGenerateMipmap(texture.m_GLTextureType);
			OpenGLGraphicsModule::LogGLError(glGetError());
		}

		glBindTexture(texture.m_GLTextureType, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());

		return handle;
	}

	TextureHandle OpenGLDevice::CreateTexture(const TextureCreateInfo& textureInfo, const void* pixels, size_t)
	{
		TextureHandle handle;
		GL_Texture& texture = m_Textures.Emplace(handle, GL_Texture());
		texture.m_Width = textureInfo.m_Width;
		texture.m_Height = textureInfo.m_Height;

		texture.m_GLTextureType = GetGLImageType(textureInfo.m_ImageType);

		texture.m_GLFormat = Formats.at(textureInfo.m_PixelFormat);
		texture.m_GLInternalFormat = textureInfo.m_PixelFormat == PixelFormat::PF_Stencil ? GL_STENCIL_INDEX8 :
			Formats.at(textureInfo.m_InternalFormat);
		texture.m_GLDataType = Datatypes.at(textureInfo.m_Type);

		glGenTextures(1, &texture.m_GLTextureID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindTexture(texture.m_GLTextureType, texture.m_GLTextureID);
		OpenGLGraphicsModule::LogGLError(glGetError());

		// Initialize texture
		glTexImage2D(texture.m_GLTextureType, 0, texture.m_GLInternalFormat, textureInfo.m_Width, textureInfo.m_Height, 0, texture.m_GLFormat, texture.m_GLDataType, pixels);
		OpenGLGraphicsModule::LogGLError(glGetError());

		const SamplerSettings& sampler = textureInfo.m_SamplerSettings;
		if (sampler.MipmapMode != Filter::F_None)
		{
			glGenerateMipmap(texture.m_GLTextureType);
			OpenGLGraphicsModule::LogGLError(glGetError());
		}

		texture.m_GLMinFilter = GetMinFilter(sampler.MipmapMode, sampler.MinFilter);
		texture.m_GLMagFilter = Filters.at(sampler.MagFilter);
		texture.m_GLTextureWrapS = Texturewraps.at(sampler.AddressModeU);
		texture.m_GLTextureWrapT = Texturewraps.at(sampler.AddressModeV);
		texture.m_GLTextureWrapR = Texturewraps.at(sampler.AddressModeW);

		glTexParameteri(texture.m_GLTextureType, GL_TEXTURE_MIN_FILTER, texture.m_GLMinFilter);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameteri(texture.m_GLTextureType, GL_TEXTURE_MAG_FILTER, texture.m_GLMagFilter);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameteri(texture.m_GLTextureType, GL_TEXTURE_WRAP_S, texture.m_GLTextureWrapS);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameteri(texture.m_GLTextureType, GL_TEXTURE_WRAP_T, texture.m_GLTextureWrapT);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameteri(texture.m_GLTextureType, GL_TEXTURE_WRAP_R, texture.m_GLTextureWrapR);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameterf(texture.m_GLTextureType, GL_TEXTURE_MIN_LOD, sampler.MinLOD);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameterf(texture.m_GLTextureType, GL_TEXTURE_MAX_LOD, sampler.MaxLOD);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameterf(texture.m_GLTextureType, GL_TEXTURE_LOD_BIAS, sampler.MipLODBias);
		OpenGLGraphicsModule::LogGLError(glGetError());

		float aniso = 0.0f;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
		OpenGLGraphicsModule::LogGLError(glGetError());
		aniso = std::min(sampler.MaxAnisotropy, aniso);
		glTexParameterf(texture.m_GLTextureType, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
		OpenGLGraphicsModule::LogGLError(glGetError());

		glBindTexture(texture.m_GLTextureType, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());

		return handle;
	}

	void OpenGLDevice::UpdateTexture(TextureHandle texture, TextureData* pTextureData)
	{
		WaitIdle();

		GL_Texture* glTexture = m_Textures.Find(texture);
		if (!glTexture)
		{
			Debug().LogError("OpenGLDevice::UpdateTexture: Invalid texture handle.");
			return;
		}

		if (glTexture->m_GLBindlessHandle)
		{
			glMakeTextureHandleNonResidentARB(glTexture->m_GLBindlessHandle);
			OpenGLGraphicsModule::LogGLError(glGetError());
		}
		glBindTexture(glTexture->m_GLTextureType, glTexture->m_GLTextureID);
		OpenGLGraphicsModule::LogGLError(glGetError());

		const SamplerSettings& sampler = pTextureData->GetSamplerSettings();
		if (sampler.MipmapMode != Filter::F_None)
		{
			glGenerateMipmap(glTexture->m_GLTextureType);
			OpenGLGraphicsModule::LogGLError(glGetError());
		}

		glTexture->m_GLMinFilter = GetMinFilter(sampler.MipmapMode, sampler.MinFilter);
		glTexture->m_GLMagFilter = Filters.at(sampler.MagFilter);
		glTexture->m_GLTextureWrapS = Texturewraps.at(sampler.AddressModeU);
		glTexture->m_GLTextureWrapT = Texturewraps.at(sampler.AddressModeV);
		glTexture->m_GLTextureWrapR = Texturewraps.at(sampler.AddressModeW);

		float aniso = 0.0f;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
		OpenGLGraphicsModule::LogGLError(glGetError());
		aniso = std::min(sampler.MaxAnisotropy, aniso);

		if (!glTexture->m_GLBindlessHandle)
		{
			glTexParameteri(glTexture->m_GLTextureType, GL_TEXTURE_MIN_FILTER, glTexture->m_GLMinFilter);
			OpenGLGraphicsModule::LogGLError(glGetError());
			glTexParameteri(glTexture->m_GLTextureType, GL_TEXTURE_MAG_FILTER, glTexture->m_GLMagFilter);
			OpenGLGraphicsModule::LogGLError(glGetError());
			glTexParameteri(glTexture->m_GLTextureType, GL_TEXTURE_WRAP_S, glTexture->m_GLTextureWrapS);
			OpenGLGraphicsModule::LogGLError(glGetError());
			glTexParameteri(glTexture->m_GLTextureType, GL_TEXTURE_WRAP_T, glTexture->m_GLTextureWrapT);
			OpenGLGraphicsModule::LogGLError(glGetError());
			glTexParameteri(glTexture->m_GLTextureType, GL_TEXTURE_WRAP_R, glTexture->m_GLTextureWrapR);
			OpenGLGraphicsModule::LogGLError(glGetError());
			glTexParameterf(glTexture->m_GLTextureType, GL_TEXTURE_MIN_LOD, sampler.MinLOD);
			OpenGLGraphicsModule::LogGLError(glGetError());
			glTexParameterf(glTexture->m_GLTextureType, GL_TEXTURE_MAX_LOD, sampler.MaxLOD);
			OpenGLGraphicsModule::LogGLError(glGetError());
			glTexParameterf(glTexture->m_GLTextureType, GL_TEXTURE_LOD_BIAS, sampler.MipLODBias);
			OpenGLGraphicsModule::LogGLError(glGetError());
			glTexParameterf(glTexture->m_GLTextureType, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
			OpenGLGraphicsModule::LogGLError(glGetError());
		}
		else
		{
			glCreateSamplers(1, &glTexture->m_GLSamplerID);
			glSamplerParameteri(glTexture->m_GLSamplerID, GL_TEXTURE_MIN_FILTER, glTexture->m_GLMinFilter);
			OpenGLGraphicsModule::LogGLError(glGetError());
			glSamplerParameteri(glTexture->m_GLSamplerID, GL_TEXTURE_MAG_FILTER, glTexture->m_GLMagFilter);
			OpenGLGraphicsModule::LogGLError(glGetError());
			glSamplerParameteri(glTexture->m_GLSamplerID, GL_TEXTURE_WRAP_S, glTexture->m_GLTextureWrapS);
			OpenGLGraphicsModule::LogGLError(glGetError());
			glSamplerParameteri(glTexture->m_GLSamplerID, GL_TEXTURE_WRAP_T, glTexture->m_GLTextureWrapT);
			OpenGLGraphicsModule::LogGLError(glGetError());
			glSamplerParameteri(glTexture->m_GLSamplerID, GL_TEXTURE_WRAP_R, glTexture->m_GLTextureWrapR);
			OpenGLGraphicsModule::LogGLError(glGetError());
			glSamplerParameterf(glTexture->m_GLSamplerID, GL_TEXTURE_MIN_LOD, sampler.MinLOD);
			OpenGLGraphicsModule::LogGLError(glGetError());
			glSamplerParameterf(glTexture->m_GLSamplerID, GL_TEXTURE_MAX_LOD, sampler.MaxLOD);
			OpenGLGraphicsModule::LogGLError(glGetError());
			glSamplerParameterf(glTexture->m_GLSamplerID, GL_TEXTURE_LOD_BIAS, sampler.MipLODBias);
			OpenGLGraphicsModule::LogGLError(glGetError());
			glSamplerParameterf(glTexture->m_GLSamplerID, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
			OpenGLGraphicsModule::LogGLError(glGetError());

			glTexture->m_GLBindlessHandle = glGetTextureSamplerHandleARB(glTexture->m_GLTextureID, glTexture->m_GLSamplerID);
			glMakeTextureHandleResidentARB(glTexture->m_GLBindlessHandle);
		}

		glBindTexture(glTexture->m_GLTextureType, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void OpenGLDevice::ReadTexturePixels(TextureHandle texture, void* dst, size_t offset, size_t size)
	{
		GL_Texture* glTexture = m_Textures.Find(texture);
		if (!glTexture)
		{
			Debug().LogError("OpenGLDevice::ReadTexturePixels: Invalid texture handle.");
			return;
		}

		glGetTextureImage(glTexture->m_GLTextureID, 0, glTexture->m_GLFormat, glTexture->m_GLDataType, size, dst);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	uint64_t OpenGLDevice::GetTextureBindlessHandle(TextureHandle texture)
	{
		GL_Texture* glTexture = m_Textures.Find(texture);
		if (!glTexture)
		{
			Debug().LogError("OpenGLDevice::ReadTexturePixels: Invalid texture handle.");
			return 0ull;
		}
		return glTexture->m_GLBindlessHandle;
	}

	RenderTextureHandle OpenGLDevice::CreateRenderTexture(RenderPassHandle renderPass, RenderTextureCreateInfo&& info)
	{
		if (info.Width == 0 || info.Height == 0)
		{
			Debug().LogError("OpenGLDevice::CreateRenderTexture: Invalid RenderTexture size.");
			return NULL;
		}

		RenderTextureHandle handle;
		GL_RenderTexture& renderTexture = m_RenderTextures.Emplace(handle, GL_RenderTexture());
		renderTexture.m_RenderPass = renderPass;
		renderTexture.m_Info = std::move(info);
		CreateRenderTexture(renderTexture);

		return handle;
	}

	TextureHandle OpenGLDevice::GetRenderTextureAttachment(RenderTextureHandle renderTexture, size_t index)
	{
		GL_RenderTexture* glRenderTexture = m_RenderTextures.Find(renderTexture);
		if (!glRenderTexture)
		{
			Debug().LogError("OpenGLDevice::GetRenderTextureAttatchment: Invalid render texture handle");
			return NULL;
		}

		if (index >= glRenderTexture->m_Textures.size())
		{
			Debug().LogError("OpenGLDevice::GetRenderTextureAttatchment: Invalid attachment index");
			return NULL;
		}
		return glRenderTexture->m_Textures[index];
	}

	void OpenGLDevice::ResizeRenderTexture(RenderTextureHandle renderTexture, uint32_t width, uint32_t height)
	{
		GL_RenderTexture* glRenderTexture = m_RenderTextures.Find(renderTexture);
		if (!glRenderTexture)
		{
			Debug().LogError("VulkanDevice::ResizeRenderTexture: Invalid render texture handle");
			return;
		}

		for (size_t i = 0; i < glRenderTexture->m_Textures.size(); ++i)
		{
			FreeTexture(glRenderTexture->m_Textures[i]);
		}
		glRenderTexture->m_Textures.clear();
		glRenderTexture->m_AttachmentNames.clear();
		glRenderTexture->m_Info.Width = width;
		glRenderTexture->m_Info.Height = height;
		glDeleteFramebuffers(1, &glRenderTexture->m_GLFramebufferID);
		glRenderTexture->m_GLFramebufferID = 0;
		CreateRenderTexture(*glRenderTexture);
	}

	RenderPassHandle OpenGLDevice::CreateRenderPass(RenderPassInfo&& info)
	{
		if (info.RenderTextureInfo.Width == 0 || info.RenderTextureInfo.Height == 0)
		{
			Debug().LogError("OpenGLDevice::CreateRenderPass: Invalid RenderTexture size.");
			return NULL;
		}

		RenderPassHandle handle;
		GL_RenderPass& renderPass = m_RenderPasses.Emplace(handle, GL_RenderPass());
		renderPass.m_RenderTexture = info.RenderTexture ? info.RenderTexture : (info.m_CreateRenderTexture ?
			CreateRenderTexture(handle, std::move(info.RenderTextureInfo)) : nullptr);
		renderPass.m_ClearColor = std::move(info.m_ClearColor);
		renderPass.m_DepthClear = info.m_DepthClear;
		renderPass.m_StencilClear = info.m_StencilClear;
		renderPass.m_Clear = info.m_LoadOp == RenderPassLoadOp::OP_Clear;

		if (info.m_CreateRenderTexture && !renderPass.m_RenderTexture)
		{
			m_RenderPasses.Erase(handle);
			Debug().LogError("OpenGLDevice::CreateRenderPass: Failed to create RenderTexture for RenderPass.");
			return NULL;
		}

		return handle;
	}

	RenderTextureHandle OpenGLDevice::GetRenderPassRenderTexture(RenderPassHandle renderPass)
	{
		GL_RenderPass* glRenderPass = m_RenderPasses.Find(renderPass);
		if (!glRenderPass)
		{
			Debug().LogError("OpenGLDevice::GetRenderPassRenderTexture: Invalid render pass handle");
			return NULL;
		}
		return glRenderPass->m_RenderTexture;
	}

	void OpenGLDevice::SetRenderPassClear(RenderPassHandle renderPass, const glm::vec4& color, float depth, uint8_t stencil)
	{
		GL_RenderPass* glRenderPass = m_RenderPasses.Find(renderPass);
		if (!glRenderPass)
		{
			Debug().LogError("OpenGLDevice::SetRenderPassClear: Invalid render pass handle");
			return;
		}
		glRenderPass->m_ClearColor = color;
		glRenderPass->m_DepthClear = depth;
		glRenderPass->m_StencilClear = stencil;
	}

	ShaderHandle OpenGLDevice::CreateShader(const FileData* pShaderFileData, const ShaderType& shaderType, const std::string& function)
	{
		ShaderHandle handle;
		GL_Shader& shader = m_Shaders.Emplace(handle, GL_Shader());
		
		const char* shaderSource = pShaderFileData->Data();

		shader.m_GLShaderType = GetShaderStageFlag(shaderType);
		shader.m_GLShaderID = glCreateShader(shader.m_GLShaderType);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glShaderSource(shader.m_GLShaderID, 1, &shaderSource, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glCompileShader(shader.m_GLShaderID);
		OpenGLGraphicsModule::LogGLError(glGetError());

		int success;
		char infoLog[512];
		glGetShaderiv(shader.m_GLShaderID, GL_COMPILE_STATUS, &success);
		OpenGLGraphicsModule::LogGLError(glGetError());

		if (!success)
		{
			glGetShaderInfoLog(shader.m_GLShaderID, 512, NULL, infoLog);
			Debug().LogError("OpenGLDevice::CreateShader: Failed to create shader.");
			OpenGLGraphicsModule::LogGLError(glGetError());
			Debug().LogError(infoLog);
		}

		return handle;
	}

	uint32_t GetGLCullFace(CullFace cullFace)
	{
		switch (cullFace)
		{
		case Glory::CullFace::None:
			return GL_NONE;
		case Glory::CullFace::Front:
			return GL_FRONT;
		case Glory::CullFace::Back:
			return GL_BACK;
		case Glory::CullFace::FrontAndBack:
			return GL_FRONT_AND_BACK;
		default:
			return GL_NONE;
		}
	}

	GLenum GtGLBlendFactor(BlendFactor blendFactor)
	{
		switch (blendFactor)
		{
		case Glory::BlendFactor::Zero:
			return GL_ZERO;
		case Glory::BlendFactor::One:
			return GL_ONE;
		case Glory::BlendFactor::SrcColor:
			return GL_SRC_COLOR;
		case Glory::BlendFactor::OneMinusSrcColor:
			return GL_ONE_MINUS_SRC_COLOR;
		case Glory::BlendFactor::DstColor:
			return GL_DST_COLOR;
		case Glory::BlendFactor::OneMinusDstColor:
			return GL_ONE_MINUS_DST_COLOR;
		case Glory::BlendFactor::SrcAlpha:
			return GL_SRC_ALPHA;
		case Glory::BlendFactor::OneMinusSrcAlpha:
			return GL_ONE_MINUS_SRC_ALPHA;
		case Glory::BlendFactor::DstAlpha:
			return GL_DST_ALPHA;
		case Glory::BlendFactor::OneMinusDstAlpha:
			return GL_ONE_MINUS_DST_ALPHA;
		case Glory::BlendFactor::ConstantColor:
			return GL_CONSTANT_COLOR;
		case Glory::BlendFactor::OneMinusConstantColor:
			return GL_ONE_MINUS_CONSTANT_COLOR;
		case Glory::BlendFactor::ConstantAlpha:
			return GL_CONSTANT_ALPHA;
		case Glory::BlendFactor::OneMinusConstantAlpha:
			return GL_ONE_MINUS_CONSTANT_ALPHA;
		case Glory::BlendFactor::SrcAlphaSaturate:
			return GL_SRC_ALPHA_SATURATE;
		case Glory::BlendFactor::Src1Color:
			return GL_SRC1_COLOR;
		case Glory::BlendFactor::OneMinusSrc1Color:
			return GL_ONE_MINUS_SRC1_COLOR;
		case Glory::BlendFactor::Src1Alpha:
			return GL_SRC1_ALPHA;
		case Glory::BlendFactor::OneMinusSrc1Alpha:
			return GL_ONE_MINUS_SRC1_ALPHA;
		default:
			return GL_ZERO;
		}
	}

	GLenum GetGLBlendOp(BlendOp blendOp)
	{
		switch (blendOp)
		{
		case Glory::BlendOp::Add:
			return GL_FUNC_ADD;
		case Glory::BlendOp::Subtract:
			return GL_FUNC_SUBTRACT;
		case Glory::BlendOp::ReverseSubtract:
			return GL_FUNC_REVERSE_SUBTRACT;
		case Glory::BlendOp::Min:
			return GL_MIN;
		case Glory::BlendOp::Max:
			return GL_MAX;
		default:
			return GL_ADD;
		}
	}

	PipelineHandle OpenGLDevice::CreatePipeline(RenderPassHandle renderPass, PipelineData* pPipeline,
		std::vector<DescriptorSetLayoutHandle>&&, size_t, const std::vector<AttributeType>&)
	{
		PipelineManager& pipelines = m_pModule->GetEngine()->GetPipelineManager();

		GL_RenderPass* glRenderPass = m_RenderPasses.Find(renderPass);
		if (!glRenderPass)
		{
			Debug().LogError("OpenGLDevice::CreatePipeline: Invalid render pass handle.");
			return NULL;
		}

		PipelineHandle handle;
		GL_Pipeline& pipeline = m_Pipelines.Emplace(handle, GL_Pipeline());
		pipeline.m_RenderPass = renderPass;
		pipeline.m_GLCullFace = GetGLCullFace(pPipeline->GetCullFace());
		pipeline.m_GLPrimitiveType = PrimitiveTypes.at(pPipeline->GetPrimitiveType());
		pipeline.m_SettingToggles = pPipeline->SettingsTogglesBitSet();
		pipeline.m_GLDepthFunc = CompareOps.at(pPipeline->GetDepthCompareOp());
		pipeline.m_GLStencilCompareOp = CompareOps.at(pPipeline->GetStencilCompareOp());
		pipeline.m_GLStencilFailOp = GLFuncs.at(pPipeline->GetStencilFailOp());
		pipeline.m_GLStencilDepthFailOp = GLFuncs.at(pPipeline->GetStencilDepthFailOp());
		pipeline.m_GLStencilPassOp = GLFuncs.at(pPipeline->GetStencilPassOp());

		pipeline.m_GLSrcColorBlendFactor = GtGLBlendFactor(pPipeline->SrcColorBlendFactor());
		pipeline.m_GLDstColorBlendFactor = GtGLBlendFactor(pPipeline->DstColorBlendFactor());
		pipeline.m_GLColorBlendOp = GetGLBlendOp(pPipeline->ColorBlendOp());
		pipeline.m_GLSrcAlphaBlendFactor = GtGLBlendFactor(pPipeline->SrcAlphaBlendFactor());
		pipeline.m_GLDstAlphaBlendFactor = GtGLBlendFactor(pPipeline->DstAlphaBlendFactor());
		pipeline.m_GLAlphaBlendOp = GetGLBlendOp(pPipeline->AlphaBlendOp());
		pipeline.m_BlendConstants = pPipeline->BlendConstants();

		if (!CreatePipeline(pipeline, pPipeline))
		{
			Debug().LogError("OpenGLDevice::CreatePipeline: Failed to create pipeline.");
			return NULL;
		}

		return handle;
	}

	void OpenGLDevice::UpdatePipelineSettings(PipelineHandle pipeline, PipelineData* pPipeline)
	{
		GL_Pipeline* glPipeline = m_Pipelines.Find(pipeline);
		if (!glPipeline)
		{
			Debug().LogError("OpenGLDevice::UpdatePipelineSettings: Invalid pipeline handle.");
			return;
		}

		glPipeline->m_GLCullFace = GetGLCullFace(pPipeline->GetCullFace());
		glPipeline->m_GLPrimitiveType = PrimitiveTypes.at(pPipeline->GetPrimitiveType());
		glPipeline->m_SettingToggles = pPipeline->SettingsTogglesBitSet();
		glPipeline->m_GLDepthFunc = CompareOps.at(pPipeline->GetDepthCompareOp());
		glPipeline->m_GLStencilCompareOp = CompareOps.at(pPipeline->GetStencilCompareOp());
		glPipeline->m_GLStencilFailOp = GLFuncs.at(pPipeline->GetStencilFailOp());
		glPipeline->m_GLStencilDepthFailOp = GLFuncs.at(pPipeline->GetStencilDepthFailOp());
		glPipeline->m_GLStencilPassOp = GLFuncs.at(pPipeline->GetStencilPassOp());

		glPipeline->m_GLSrcColorBlendFactor = GtGLBlendFactor(pPipeline->SrcColorBlendFactor());
		glPipeline->m_GLDstColorBlendFactor = GtGLBlendFactor(pPipeline->DstColorBlendFactor());
		glPipeline->m_GLColorBlendOp = GetGLBlendOp(pPipeline->ColorBlendOp());
		glPipeline->m_GLSrcAlphaBlendFactor = GtGLBlendFactor(pPipeline->SrcAlphaBlendFactor());
		glPipeline->m_GLDstAlphaBlendFactor = GtGLBlendFactor(pPipeline->DstAlphaBlendFactor());
		glPipeline->m_GLAlphaBlendOp = GetGLBlendOp(pPipeline->AlphaBlendOp());
		glPipeline->m_BlendConstants = pPipeline->BlendConstants();
	}

	void OpenGLDevice::RecreatePipeline(PipelineHandle pipeline, PipelineData* pPipeline)
	{
		GL_Pipeline* glPipeline = m_Pipelines.Find(pipeline);
		if (!glPipeline)
		{
			Debug().LogError("OpenGLDevice::RecreatePipeline: Invalid pipeline handle.");
			return;
		}

		glPipeline->m_GLCullFace = GetGLCullFace(pPipeline->GetCullFace());
		glPipeline->m_GLPrimitiveType = PrimitiveTypes.at(pPipeline->GetPrimitiveType());
		glPipeline->m_SettingToggles = pPipeline->SettingsTogglesBitSet();
		glPipeline->m_GLDepthFunc = CompareOps.at(pPipeline->GetDepthCompareOp());
		glPipeline->m_GLStencilCompareOp = CompareOps.at(pPipeline->GetStencilCompareOp());
		glPipeline->m_GLStencilFailOp = GLFuncs.at(pPipeline->GetStencilFailOp());
		glPipeline->m_GLStencilDepthFailOp = GLFuncs.at(pPipeline->GetStencilDepthFailOp());
		glPipeline->m_GLStencilPassOp = GLFuncs.at(pPipeline->GetStencilPassOp());

		glPipeline->m_GLSrcColorBlendFactor = GtGLBlendFactor(pPipeline->SrcColorBlendFactor());
		glPipeline->m_GLDstColorBlendFactor = GtGLBlendFactor(pPipeline->DstColorBlendFactor());
		glPipeline->m_GLColorBlendOp = GetGLBlendOp(pPipeline->ColorBlendOp());
		glPipeline->m_GLSrcAlphaBlendFactor = GtGLBlendFactor(pPipeline->SrcAlphaBlendFactor());
		glPipeline->m_GLDstAlphaBlendFactor = GtGLBlendFactor(pPipeline->DstAlphaBlendFactor());
		glPipeline->m_GLAlphaBlendOp = GetGLBlendOp(pPipeline->AlphaBlendOp());
		glPipeline->m_BlendConstants = pPipeline->BlendConstants();

		for (auto& shader : glPipeline->m_Shaders)
		{
			GL_Shader* glShader = m_Shaders.Find(shader);
			if (!glShader) continue;
			glDetachShader(glPipeline->m_GLProgramID, glShader->m_GLShaderID);
			OpenGLGraphicsModule::LogGLError(glGetError());
			FreeShader(shader);
		}
		glPipeline->m_Shaders.clear();

		if (!CreatePipeline(*glPipeline, pPipeline))
			Debug().LogError("OpenGLDevice::RecreatePipeline: Failed to create pipeline.");
	}

	PipelineHandle OpenGLDevice::CreateComputePipeline(PipelineData* pPipeline, std::vector<DescriptorSetLayoutHandle>&& descriptorSetLayouts)
	{
		PipelineManager& pipelines = m_pModule->GetEngine()->GetPipelineManager();

		PipelineHandle handle;
		GL_Pipeline& pipeline = m_Pipelines.Emplace(handle, GL_Pipeline());
		pipeline.m_RenderPass = NULL;

		int success;
		char infoLog[512];

		pipeline.m_GLProgramID = glCreateProgram();
		OpenGLGraphicsModule::LogGLError(glGetError());

		for (size_t i = 0; i < pPipeline->ShaderCount(); ++i)
		{
			const FileData* pShader = pPipeline->Shader(pipelines, i);
			const ShaderType type = pPipeline->GetShaderType(pipelines, i);
			ShaderHandle shaderHandle = AcquireCachedShader(pShader, type, "main");
			if (!shaderHandle)
			{
				Debug().LogError("OpenGLDevice::CreatePipeline: Invalid render pass handle.");
				continue;
			}

			GL_Shader* shader = m_Shaders.Find(shaderHandle);

			glAttachShader(pipeline.m_GLProgramID, shader->m_GLShaderID);
			OpenGLGraphicsModule::LogGLError(glGetError());
		}

		glLinkProgram(pipeline.m_GLProgramID);
		OpenGLGraphicsModule::LogGLError(glGetError());

		glGetProgramiv(pipeline.m_GLProgramID, GL_LINK_STATUS, &success);
		OpenGLGraphicsModule::LogGLError(glGetError());
		if (!success)
		{
			glGetProgramInfoLog(pipeline.m_GLProgramID, 512, NULL, infoLog);
			OpenGLGraphicsModule::LogGLError(glGetError());
			Debug().LogError(infoLog);
		}

		return handle;
	}

	DescriptorSetLayoutHandle OpenGLDevice::CreateDescriptorSetLayout(DescriptorSetLayoutInfo&& setLayoutInfo)
	{
		auto iter = m_CachedDescriptorSetLayouts.find(setLayoutInfo);
		if (iter == m_CachedDescriptorSetLayouts.end())
		{
			iter = m_CachedDescriptorSetLayouts.emplace(setLayoutInfo, UUID()).first;
			GL_DescriptorSetLayout& setLayout = m_SetLayouts.Emplace(iter->second, GL_DescriptorSetLayout());
			for (size_t i = 0; i < setLayoutInfo.m_Buffers.size(); ++i)
			{
				setLayout.m_BindingIndices.emplace_back(setLayoutInfo.m_Buffers[i].m_BindingIndex);
				setLayout.m_DescriptorCounts.emplace_back(0u);
			}
			for (size_t i = 0; i < setLayoutInfo.m_Samplers.size(); ++i)
			{
				setLayout.m_BindingIndices.emplace_back(setLayoutInfo.m_Samplers[i].m_BindingIndex);
				setLayout.m_DescriptorCounts.emplace_back(setLayoutInfo.m_Samplers[i].m_SamplerCount);
			}
			setLayout.m_SamplerNames = std::move(setLayoutInfo.m_SamplerNames);
		}
		return iter->second;
	}

	DescriptorSetHandle OpenGLDevice::CreateDescriptorSet(DescriptorSetInfo&& setInfo)
	{
		GL_DescriptorSetLayout* glSetLayout = m_SetLayouts.Find(setInfo.m_Layout);
		if (!glSetLayout)
		{
			Debug().LogError("OpenGLDevice::CreateDescriptorSet: Invalid set layout handle.");
			return NULL;
		}

		std::vector<BufferHandle> bufferHandles(setInfo.m_Buffers.size());
		std::vector<TextureHandle> textureHandles(glSetLayout->m_SamplerNames.size());

		for (size_t i = 0; i < setInfo.m_Buffers.size(); ++i)
		{
			GL_Buffer* glBuffer = m_Buffers.Find(setInfo.m_Buffers[i].m_BufferHandle);
			if (!glBuffer)
			{
				Debug().LogError("OpenGLDevice::CreateDescriptorSet: Invalid buffer handle.");
				return NULL;
			}

			bufferHandles[i] = setInfo.m_Buffers[i].m_BufferHandle;
		}

		for (size_t i = 0; i < setInfo.m_Samplers.size(); ++i)
		{
			GL_Texture* glTexture = m_Textures.Find(setInfo.m_Samplers[i].m_TextureHandle);
			if (!glTexture)
			{
				textureHandles[i] = NULL;
				continue;
			}

			textureHandles[i] = setInfo.m_Samplers[i].m_TextureHandle;
		}

		DescriptorSetHandle handle;
		GL_DescriptorSet& set = m_Sets.Emplace(handle, GL_DescriptorSet());

		set.m_BindlessTexturesBuffers.resize(glSetLayout->m_DescriptorCounts.size(), 0u);
		for (size_t i = 0; i < set.m_BindlessTexturesBuffers.size(); ++i)
		{
			const size_t descriptorCount = glSetLayout->m_DescriptorCounts[i];
			if (descriptorCount <= 1) continue;
			set.m_BindlessTexturesBuffers[i] = CreateBuffer(sizeof(uint64_t)*descriptorCount, BT_Storage, BF_None);
			std::vector<uint64_t> clearValues(descriptorCount, 0ull);
			AssignBuffer(set.m_BindlessTexturesBuffers[i], clearValues.data());
			textureHandles[i] = nullptr;
		}

		set.m_Buffers = std::move(bufferHandles);
		set.m_Textures = std::move(textureHandles);
		set.m_Layout = setInfo.m_Layout;

		return handle;
	}

	void OpenGLDevice::UpdateDescriptorSet(DescriptorSetHandle descriptorSet, const DescriptorSetUpdateInfo& setWriteInfo)
	{
		GL_DescriptorSet* glSet = m_Sets.Find(descriptorSet);
		if (!glSet)
		{
			Debug().LogError("OpenGLDevice::UpdateDescriptorSet: Invalid set handle.");
			return;
		}

		GL_DescriptorSetLayout* glSetLayout = m_SetLayouts.Find(glSet->m_Layout);
		if (!glSetLayout)
		{
			Debug().LogError("OpenGLDevice::UpdateDescriptorSet: Invalid set layout handle.");
			return;
		}

		for (size_t i = 0; i < setWriteInfo.m_Buffers.size(); ++i)
		{
			auto& bufferInfo = setWriteInfo.m_Buffers[i];
			glSet->m_Buffers[bufferInfo.m_DescriptorIndex] = bufferInfo.m_BufferHandle;
		}

		for (size_t i = 0; i < setWriteInfo.m_Samplers.size(); ++i)
		{
			auto& samplerInfo = setWriteInfo.m_Samplers[i];
			const size_t index = samplerInfo.m_DescriptorIndex - setWriteInfo.m_Buffers.size();
			if (glSet->m_BindlessTexturesBuffers[index])
			{
				std::vector<uint64_t> textureHandles(samplerInfo.m_DescriptorCount);
				for (size_t i = 0; i < samplerInfo.m_DescriptorCount; ++i)
				{
					TextureHandle texture = samplerInfo.m_TextureHandles[i];
					GL_Texture* glTexture = m_Textures.Find(texture);
					if (!texture || !glTexture)
					{
						texture = m_DefaultTexture;
						glTexture = m_Textures.Find(texture);
					}

					textureHandles[i] = glTexture->m_GLBindlessHandle;
				}

				AssignBuffer(glSet->m_BindlessTexturesBuffers[index], textureHandles.data(), textureHandles.size()*sizeof(uint64_t));
				glSet->m_Textures[index] = nullptr;
				continue;
			}
			glSet->m_Textures[index] = *samplerInfo.m_TextureHandles;
		}
	}

	SwapchainHandle OpenGLDevice::CreateSwapchain(Window* pWindow, bool vsync, uint32_t minImageCount)
	{
		SwapchainHandle handle;
		GL_Swapchain& swapchain = m_Swapchains.Emplace(handle, GL_Swapchain());
		swapchain.m_pWindow = pWindow;
		pWindow->SetGLSwapInterval(vsync ? 1 : 0);

		int width, height;
		pWindow->GetDrawableSize(&width, &height);

		/* Swap chain emulation */
		minImageCount = 1;
		swapchain.m_SwapchainImages.resize(minImageCount);

		for (size_t i = 0; i < minImageCount; ++i)
		{
			RenderTextureCreateInfo info;
			info.HasDepth = false;
			info.HasStencil = false;
			info.EnableDepthStencilSampling = false;
			info.Width = uint32_t(width);
			info.Height = uint32_t(height);
			info.Attachments.push_back(Attachment("Color", PixelFormat::PF_RGBA,
				PixelFormat::PF_R8G8B8A8Srgb, ImageType::IT_2D, ImageAspect::IA_Color, DataType::DT_UByte));
			swapchain.m_SwapchainImages[i] = CreateRenderTexture(NULL, std::move(info));
		}

		std::stringstream str;
		str << "OpenGLDevice: Swap chain " << handle << " created.";
		Debug().LogInfo(str.str());

		return handle;
	}

	uint32_t OpenGLDevice::GetSwapchainImageCount(SwapchainHandle swapchain)
	{
		GL_Swapchain* glSwapchain = m_Swapchains.Find(swapchain);
		if (!glSwapchain)
		{
			Debug().LogError("OpenGLDevice::GetSwapchainImageCount: Invalid swapchain handle.");
			return 0;
		}
		return static_cast<uint32_t>(glSwapchain->m_SwapchainImages.size());
	}

	TextureHandle OpenGLDevice::GetSwapchainImage(SwapchainHandle swapchain, uint32_t imageIndex)
	{
		GL_Swapchain* glSwapchain = m_Swapchains.Find(swapchain);
		if (!glSwapchain)
		{
			Debug().LogError("OpenGLDevice::GetSwapchainImageCount: Invalid swapchain handle.");
			return NULL;
		}
		return GetRenderTextureAttachment(glSwapchain->m_SwapchainImages[imageIndex], 0);
	}

	void OpenGLDevice::RecreateSwapchain(SwapchainHandle swapchain)
	{
	}

	SemaphoreHandle OpenGLDevice::CreateSemaphore()
	{
		return NULL;
	}

	void OpenGLDevice::FreeBuffer(BufferHandle& handle)
	{
		GL_Buffer* buffer = m_Buffers.Find(handle);
		if (!buffer)
		{
			Debug().LogError("OpenGLDevice::FreeBuffer: Invalid buffer handle.");
			return;
		}

		glDeleteBuffers(1, &buffer->m_GLBufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		m_Buffers.Erase(handle);

		handle = 0;
	}

	void OpenGLDevice::FreeMesh(MeshHandle& handle)
	{
		GL_Mesh* mesh = m_Meshes.Find(handle);
		if (!mesh)
		{
			Debug().LogError("OpenGLDevice::FreeMesh: Invalid mesh handle.");
			return;
		}

		glDeleteVertexArrays(1, &mesh->m_GLVertexArrayID);
		OpenGLGraphicsModule::LogGLError(glGetError());

		for (auto& buffer : mesh->m_Buffers)
			FreeBuffer(buffer);

		m_Meshes.Erase(handle);

		handle = 0;
	}

	void OpenGLDevice::FreeTexture(TextureHandle& handle)
	{
		GL_Texture* texture = m_Textures.Find(handle);
		if (!texture)
		{
			Debug().LogError("OpenGLDevice::FreeTexture: Invalid texture handle.");
			return;
		}

		glDeleteTextures(1, &texture->m_GLTextureID);
		OpenGLGraphicsModule::LogGLError(glGetError());

		m_Textures.Erase(handle);

		handle = 0;
	}

	void OpenGLDevice::FreeRenderTexture(RenderTextureHandle& handle)
	{
		GL_RenderTexture* renderTexture = m_RenderTextures.Find(handle);
		if (!renderTexture)
		{
			Debug().LogError("OpenGLDevice::FreeRenderTexture: Invalid render texture handle.");
			return;
		}

		glDeleteFramebuffers(1, &renderTexture->m_GLFramebufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());

		for (auto texture : renderTexture->m_Textures)
		{
			FreeTexture(texture);
		}

		renderTexture->m_Textures.clear();
		renderTexture->m_AttachmentNames.clear();

		m_RenderTextures.Erase(handle);

		handle = 0;
	}

	void OpenGLDevice::FreeRenderPass(RenderPassHandle& handle)
	{
		GL_RenderPass* renderPass = m_RenderPasses.Find(handle);
		if (!renderPass)
		{
			Debug().LogError("OpenGLDevice::FreeRenderPass: Invalid render pass handle.");
			return;
		}

		FreeRenderTexture(renderPass->m_RenderTexture);

		m_RenderPasses.Erase(handle);

		handle = 0;
	}

	void OpenGLDevice::FreeShader(ShaderHandle& handle)
	{
		GL_Shader* shader = m_Shaders.Find(handle);
		if (!shader)
		{
			Debug().LogError("OpenGLDevice::FreeShader: Invalid shader handle.");
			return;
		}

		glDeleteShader(shader->m_GLShaderID);
		OpenGLGraphicsModule::LogGLError(glGetError());

		m_Shaders.Erase(handle);

		handle = 0;
	}

	void OpenGLDevice::FreePipeline(PipelineHandle& handle)
	{
		GL_Pipeline* pipeline = m_Pipelines.Find(handle);
		if (!pipeline)
		{
			Debug().LogError("OpenGLDevice::FreePipeline: Invalid pipeline handle.");
			return;
		}

		glDeleteProgram(pipeline->m_GLProgramID);
		OpenGLGraphicsModule::LogGLError(glGetError());

		m_Pipelines.Erase(handle);

		handle = 0;
	}

	void OpenGLDevice::FreeDescriptorSetLayout(DescriptorSetLayoutHandle& handle)
	{
		GL_DescriptorSetLayout* glSetLayout = m_SetLayouts.Find(handle);
		if (!glSetLayout)
		{
			Debug().LogError("OpenGLDevice::FreeDescriptorSetLayout: Invalid set layout handle.");
			return;
		}

		m_SetLayouts.Erase(handle);

		handle = 0;
	}

	void OpenGLDevice::FreeDescriptorSet(DescriptorSetHandle& handle)
	{
		GL_DescriptorSet* glSet = m_Sets.Find(handle);
		if (!glSet)
		{
			Debug().LogError("OpenGLDevice::FreeDescriptorSet: Invalid set handle.");
			return;
		}

		m_Sets.Erase(handle);

		handle = 0;
	}

	void OpenGLDevice::FreeSwapchain(SwapchainHandle& handle)
	{
		GL_Swapchain* glSwapchain = m_Swapchains.Find(handle);
		if (!glSwapchain)
		{
			Debug().LogError("OpenGLDevice::FreeSwapchain: Invalid swap chain handle.");
			return;
		}

		m_Swapchains.Erase(handle);

		std::stringstream str;
		str << "OpenGLDevice: Swap chain " << handle << " was freed from device memory.";
		Debug().LogInfo(str.str());

		handle = 0;
	}

	void OpenGLDevice::FreeSemaphore(SemaphoreHandle& handle)
	{
	}

	void OpenGLDevice::OnInitialize()
	{
		m_ConstantsBuffer = CreateBuffer(PushConstantsMaxSize, BT_Uniform, BF_Write);
	}

	void OpenGLDevice::CreateRenderTexture(GL_RenderTexture& renderTexture)
	{
		/* Create framebuffer */
		glGenFramebuffers(1, &renderTexture.m_GLFramebufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindFramebuffer(GL_FRAMEBUFFER, renderTexture.m_GLFramebufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());

		const size_t numAttachments = renderTexture.m_Info.Attachments.size() +
			(renderTexture.m_Info.HasDepth ? 1 : 0) + (renderTexture.m_Info.HasStencil ? 1 : 0);
		renderTexture.m_AttachmentNames.resize(numAttachments);
		renderTexture.m_Textures.resize(numAttachments);

		SamplerSettings sampler;
		sampler.MipmapMode = Filter::F_None;
		sampler.MinFilter = Filter::F_Nearest;
		sampler.MagFilter = Filter::F_Nearest;

		size_t textureCounter = 0;
		for (size_t i = 0; i < renderTexture.m_Info.Attachments.size(); ++i)
		{
			const Attachment& attachment = renderTexture.m_Info.Attachments[i];
			renderTexture.m_Textures[i] = attachment.Texture ? attachment.Texture :
				CreateTexture({ renderTexture.m_Info.Width, renderTexture.m_Info.Height,
					attachment.Format, attachment.InternalFormat, attachment.ImageType,
					attachment.m_Type, attachment.Flags, attachment.ImageAspect, sampler });
			renderTexture.m_AttachmentNames[i] = attachment.Name;
			++textureCounter;
		}

		size_t depthIndex = 0, stencilIndex = 0;
		if (renderTexture.m_Info.HasDepth)
		{
			depthIndex = textureCounter;
			renderTexture.m_Textures[depthIndex] =
				CreateTexture({ renderTexture.m_Info.Width, renderTexture.m_Info.Height, PixelFormat::PF_Depth,
					PixelFormat::PF_Depth32, ImageType::IT_2D, DataType::DT_UInt, IF_None, ImageAspect::IA_Depth, sampler });
			renderTexture.m_AttachmentNames[depthIndex] = "Depth";
			++textureCounter;
		}

		if (renderTexture.m_Info.HasStencil)
		{
			stencilIndex = textureCounter;
			renderTexture.m_Textures[stencilIndex] =
				CreateTexture({ renderTexture.m_Info.Width, renderTexture.m_Info.Height, PixelFormat::PF_Stencil,
					PixelFormat::PF_R8Uint, ImageType::IT_2D, DataType::DT_UByte, IF_None, ImageAspect::IA_Stencil, sampler });
			renderTexture.m_AttachmentNames[stencilIndex] = "Stencil";
			++textureCounter;
		}

		// Initialize the framebuffer
		const size_t attachmentCount = renderTexture.m_Info.Attachments.size();
		std::vector<GLenum> drawBuffers = std::vector<GLenum>(attachmentCount);
		for (uint32_t i = 0; i < attachmentCount; i++)
		{
			drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
			GL_Texture* glTexture = m_Textures.Find(renderTexture.m_Textures[i]);
			glFramebufferTexture(GL_FRAMEBUFFER, drawBuffers[i], glTexture->m_GLTextureID, 0);
			OpenGLGraphicsModule::LogGLError(glGetError());
		}

		if (renderTexture.m_Info.HasDepth)
		{
			GL_Texture* glTexture = m_Textures.Find(renderTexture.m_Textures[depthIndex]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, glTexture->m_GLTextureID, 0);
			OpenGLGraphicsModule::LogGLError(glGetError());
		}
		if (renderTexture.m_Info.HasStencil)
		{
			GL_Texture* glTexture = m_Textures.Find(renderTexture.m_Textures[stencilIndex]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, glTexture->m_GLTextureID, 0);
			OpenGLGraphicsModule::LogGLError(glGetError());
		}

		if (attachmentCount > 0)
		{
			glDrawBuffers(attachmentCount, &drawBuffers[0]);
			OpenGLGraphicsModule::LogGLError(glGetError());
		}

		// Check if something went wrong
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			Debug().LogError("OpenGLDevice::CreateRenderTexture: There was an error when trying to create a frame buffer.");
			return;
		}

		glBindTexture(GL_TEXTURE_2D, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindFramebuffer(GL_FRAMEBUFFER, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	bool OpenGLDevice::CreatePipeline(GL_Pipeline& pipeline, PipelineData* pPipeline)
	{
		PipelineManager& pipelines = m_pModule->GetEngine()->GetPipelineManager();

		int success;
		char infoLog[512];

		pipeline.m_GLProgramID = glCreateProgram();
		OpenGLGraphicsModule::LogGLError(glGetError());

		for (size_t i = 0; i < pPipeline->ShaderCount(); ++i)
		{
			const FileData* pShader = pPipeline->Shader(pipelines, i);
			const ShaderType type = pPipeline->GetShaderType(pipelines, i);
			ShaderHandle shaderHandle = AcquireCachedShader(pShader, type, "main");
			if (!shaderHandle)
			{
				Debug().LogError("OpenGLDevice::CreatePipeline: Invalid render pass handle.");
				continue;
			}

			GL_Shader* shader = m_Shaders.Find(shaderHandle);

			glAttachShader(pipeline.m_GLProgramID, shader->m_GLShaderID);
			OpenGLGraphicsModule::LogGLError(glGetError());
		}

		glLinkProgram(pipeline.m_GLProgramID);
		OpenGLGraphicsModule::LogGLError(glGetError());

		glGetProgramiv(pipeline.m_GLProgramID, GL_LINK_STATUS, &success);
		OpenGLGraphicsModule::LogGLError(glGetError());
		if (!success)
		{
			glGetProgramInfoLog(pipeline.m_GLProgramID, 512, NULL, infoLog);
			OpenGLGraphicsModule::LogGLError(glGetError());
			Debug().LogError(infoLog);
			return false;
		}
		return true;
	}

	void OpenGLDevice::PushCommand(GL_CommandBuffer& buffer, GL_CommandData&& commandData)
	{
		if (!m_IsCommandBufferEmulationEnabled)
		{
			/* Execute immediately */
			OpenGLCommandImpl::Command_Impl(*this, buffer, commandData);
			return;
		}

		if (buffer.m_CommandsSize > 0 && buffer.m_Commands[buffer.m_CommandsSize - 1].m_CommandType == GLCommandType::End)
		{
			Debug().LogError("OpenGLDevice::PushCommand: Command buffer recording ended.");
			return;
		}

		if (buffer.m_CommandsSize == buffer.m_CommandsCapacity)
		{
			/* Resize */
			const size_t newCapacity = buffer.m_CommandsCapacity + buffer.m_CommandsCapacity/2;
			GL_CommandData* newCommandData = new GL_CommandData[newCapacity];
			for (size_t i = 0; i < buffer.m_CommandsSize; ++i)
				newCommandData[i] = std::move(buffer.m_Commands[i]);
			buffer.m_Commands.reset(newCommandData);
			buffer.m_CommandsCapacity = newCapacity;
		}

		buffer.m_Commands[buffer.m_CommandsSize] = std::move(commandData);
		++buffer.m_CommandsSize;
	}

#pragma endregion
}
