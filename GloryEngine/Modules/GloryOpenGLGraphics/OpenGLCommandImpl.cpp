#include "OpenGLCommandImpl.h"
#include "OpenGLDevice.h"
#include "OpenGLGraphicsModule.h"
#include "OpenGLData.h"

#include <PipelineData.h>
#include <Debug.h>
#include <GloryAssert.h>

namespace Glory
{
#define COMMAND_CASES() \
	X(Begin);\
	X(BeginRenderPass);\
	X(BeginPipeline);\
	X(End);\
	X(EndRenderPass);\
	X(EndPipeline);\
	X(BindDescriptorSets);\
	X(PushConstants);\
	X(DrawMesh);\
	X(Dispatch);\
	X(SetStencilTestEnabled);\
	X(SetStencilOp);\
	X(SetStencilWriteMask);\
	X(SetViewport);\
	X(SetScissor);\
	X(PipelineBarrier);\
	X(CopyImage);\
	X(CopyImageToBuffer);

#define X(type) \
	case GLCommandType::type:\
	type##_Impl(device, commandBuffer, data);\
	break;

	void OpenGLCommandImpl::Command_Impl(OpenGLDevice& device, const GL_CommandBuffer& commandBuffer, const GL_CommandData& data)
	{
		switch (data.m_CommandType)
		{
		case GLCommandType::Unknown:
			device.Debug().LogError("OpenGLCommandImpl::Command_Impl: Unknown command type.");
			break;
			COMMAND_CASES();
		default:
			GLORY_ASSERT_UNREACHABLE_CODE();
			break;
		}
	}

#undef COMMAND_CASES
#undef X

	void OpenGLCommandImpl::Begin_Impl(OpenGLDevice& device, const GL_CommandBuffer&, const GL_CommandData&)
	{
		/* Nothing to do here */
	}

	void OpenGLCommandImpl::BeginRenderPass_Impl(OpenGLDevice& device, const GL_CommandBuffer&, const GL_CommandData& data)
	{
		GL_RenderPass* glRenderPass = device.m_RenderPasses.Find(data.m_RenderPass);
		if (!glRenderPass)
		{
			device.Debug().LogError("OpenGLCommandImpl::BeginRenderPass: Invalid render pass handle.");
			return;
		}
		GL_RenderTexture* glRenderTexture = device.m_RenderTextures.Find(glRenderPass->m_RenderTexture);
		if (!glRenderTexture)
		{
			device.Debug().LogError("OpenGLCommandImpl::BeginRenderPass: Render pass has an invalid render texture handle.");
			return;
		}

		glDisable(GL_SCISSOR_TEST);
		glBindFramebuffer(GL_FRAMEBUFFER, glRenderTexture->m_GLFramebufferID);
		glViewport(0, 0, glRenderTexture->m_Info.Width, glRenderTexture->m_Info.Height);
		OpenGLGraphicsModule::LogGLError(glGetError());

		const bool hasDepth = glRenderTexture->m_Info.HasDepth;
		const bool hasStencil = glRenderTexture->m_Info.HasStencil;
		const bool hasStencilOrDepth = hasDepth || hasStencil;
		const bool hasColor = glRenderTexture->m_Textures.size() > hasStencilOrDepth ? 1 : 0;

		glColorMask(hasColor, hasColor, hasColor, hasColor);
		glDepthMask(hasDepth);
		glStencilMask(hasStencil);

		if (!glRenderPass->m_Clear)
		{
			glClear(0);
			OpenGLGraphicsModule::LogGLError(glGetError());
			return;
		}

		if (hasColor)
			glClearColor(glRenderPass->m_ClearColor.x, glRenderPass->m_ClearColor.y, glRenderPass->m_ClearColor.z, glRenderPass->m_ClearColor.w);
		if (hasDepth)
			glClearDepth(glRenderPass->m_DepthClear);
		if (hasStencil)
			glClearStencil(glRenderPass->m_StencilClear);

		GLbitfield clearFlags = 0;
		if (hasColor)
			clearFlags |= GL_COLOR_BUFFER_BIT;
		if (hasDepth)
			clearFlags |= GL_DEPTH_BUFFER_BIT;
		if (hasStencil)
			clearFlags |= GL_STENCIL_BUFFER_BIT;

		OpenGLGraphicsModule::LogGLError(glGetError());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void OpenGLCommandImpl::BeginPipeline_Impl(OpenGLDevice& device, const GL_CommandBuffer& commandBuffer, const GL_CommandData& data)
	{
		GL_Pipeline* glPipeline = device.m_Pipelines.Find(data.m_Pipeline);
		if (!glPipeline)
		{
			device.Debug().LogError("OpenGLCommandImpl::BeginPipeline: Invalid pipeline handle.");
			return;
		}

		glUseProgram(glPipeline->m_GLProgramID);
		OpenGLGraphicsModule::LogGLError(glGetError());

		if (glPipeline->m_GLCullFace != 0)
		{
			glEnable(GL_CULL_FACE);
			glCullFace(glPipeline->m_GLCullFace);
		}
		else glDisable(GL_CULL_FACE);
		glDisable(GL_SCISSOR_TEST);

		if (glPipeline->m_SettingToggles.IsSet(PipelineData::DepthTestEnable))
		{
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(glPipeline->m_GLDepthFunc);
		}
		else
			glDisable(GL_DEPTH_TEST);
		glDepthMask(glPipeline->m_SettingToggles.IsSet(PipelineData::DepthWriteEnable));
		commandBuffer.m_GLCurrentPrimitives = glPipeline->m_GLPrimitiveType;

		const bool r = glPipeline->m_SettingToggles.IsSet(PipelineData::ColorWriteRed);
		const bool g = glPipeline->m_SettingToggles.IsSet(PipelineData::ColorWriteGreen);
		const bool b = glPipeline->m_SettingToggles.IsSet(PipelineData::ColorWriteBlue);
		const bool a = glPipeline->m_SettingToggles.IsSet(PipelineData::ColorWriteAlpha);
		glColorMask(r, g, b, a);

		if (glPipeline->m_SettingToggles.IsSet(PipelineData::BlendEnable))
		{
			glEnable(GL_BLEND);
			glBlendFuncSeparate(glPipeline->m_GLSrcColorBlendFactor, glPipeline->m_GLDstColorBlendFactor,
				glPipeline->m_GLSrcAlphaBlendFactor, glPipeline->m_GLDstAlphaBlendFactor);
			glBlendEquationSeparate(glPipeline->m_GLColorBlendOp, glPipeline->m_GLAlphaBlendOp);
			glBlendColor(glPipeline->m_BlendConstants.r, glPipeline->m_BlendConstants.g,
				glPipeline->m_BlendConstants.b, glPipeline->m_BlendConstants.a);
		}
		else
			glDisable(GL_BLEND);

		if (glPipeline->m_SettingToggles.IsSet(PipelineData::StencilTestEnable))
		{
			glEnable(GL_STENCIL_TEST);
			const uint8_t compareMask = static_cast<uint8_t>(*glPipeline->m_SettingToggles.Data() >> PipelineData::StencilCompareMaskBegin);
			const uint8_t ref = static_cast<uint8_t>(*glPipeline->m_SettingToggles.Data() >> PipelineData::StencilReferenceBegin);
			glStencilOp(glPipeline->m_GLStencilFailOp, glPipeline->m_GLStencilDepthFailOp, glPipeline->m_GLStencilPassOp);
			glStencilFunc(glPipeline->m_GLStencilCompareOp, int32_t(ref), uint32_t(compareMask));
		}
		else
			glDisable(GL_STENCIL_TEST);

		const uint8_t writeMask = static_cast<uint8_t>(*glPipeline->m_SettingToggles.Data() >> PipelineData::StencilWriteMaskBegin);
		glStencilMask(uint32_t(writeMask));
	}

	void OpenGLCommandImpl::End_Impl(OpenGLDevice& device, const GL_CommandBuffer&, const GL_CommandData&)
	{
		/* Nothing to do here */
	}

	void OpenGLCommandImpl::EndRenderPass_Impl(OpenGLDevice& device, const GL_CommandBuffer&, const GL_CommandData&)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void OpenGLCommandImpl::EndPipeline_Impl(OpenGLDevice& device, const GL_CommandBuffer&, const GL_CommandData&)
	{
		glUseProgram(NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void OpenGLCommandImpl::BindDescriptorSets_Impl(OpenGLDevice& device, const GL_CommandBuffer&, const GL_CommandData& data)
	{
		GL_Pipeline* glPipeline = device.m_Pipelines.Find(data.m_Pipeline);
		if (!glPipeline)
		{
			device.Debug().LogError("OpenGLCommandImpl::BindDescriptorSet: Invalid pipeline handle.");
			return;
		}

		GL_DescriptorSet* glSet = device.m_Sets.Find(data.m_DescriptorSet);
		if (!glSet)
		{
			device.Debug().LogError("OpenGLCommandImpl::BindDescriptorSets: Invalid set handle.");
			return;
		}

		GL_DescriptorSetLayout* glSetLayout = device.m_SetLayouts.Find(glSet->m_Layout);
		if (!glSetLayout)
		{
			device.Debug().LogError("OpenGLCommandImpl::BindDescriptorSets: Invalid set layout handle.");
			return;
		}

		size_t index = 0;
		for (size_t i = 0; i < glSet->m_Buffers.size(); ++i)
		{
			GL_Buffer* glBuffer = device.m_Buffers.Find(glSet->m_Buffers[i]);
			if (!glBuffer)
			{
				device.Debug().LogError("OpenGLCommandImpl::BindDescriptorSet: Invalid buffer handle.");
				return;
			}

			glBindBufferBase(glBuffer->m_GLTarget, (GLuint)glSetLayout->m_BindingIndices[index], glBuffer->m_GLBufferID);
			OpenGLGraphicsModule::LogGLError(glGetError());
			++index;
		}

		for (size_t i = 0; i < glSet->m_Textures.size(); ++i)
		{
			if (glSet->m_BindlessTexturesBuffers[i])
			{
				GL_Buffer* glBuffer = device.m_Buffers.Find(glSet->m_BindlessTexturesBuffers[i]);
				glBindBufferBase(glBuffer->m_GLTarget, (GLuint)glSetLayout->m_BindingIndices[index], glBuffer->m_GLBufferID);
				OpenGLGraphicsModule::LogGLError(glGetError());
				++index;
				continue;
			}

			GL_Texture* glTexture = device.m_Textures.Find(glSet->m_Textures[i]);
			GLuint texLocation = glGetUniformLocation(glPipeline->m_GLProgramID, glSetLayout->m_SamplerNames[i].c_str());
			OpenGLGraphicsModule::LogGLError(glGetError());
			glUniform1i(texLocation, glSetLayout->m_BindingIndices[index]);
			OpenGLGraphicsModule::LogGLError(glGetError());

			glActiveTexture(GL_TEXTURE0 + glSetLayout->m_BindingIndices[index]);
			OpenGLGraphicsModule::LogGLError(glGetError());
			glBindTexture(glTexture ? glTexture->m_GLTextureType : GL_TEXTURE_2D, glTexture ? glTexture->m_GLTextureID : 0);
			OpenGLGraphicsModule::LogGLError(glGetError());

			glActiveTexture(GL_TEXTURE0);
			OpenGLGraphicsModule::LogGLError(glGetError());
			++index;
		}
	}

	void OpenGLCommandImpl::PushConstants_Impl(OpenGLDevice& device, const GL_CommandBuffer& commandBuffer, const GL_CommandData& data)
	{
		GL_Buffer* glBuffer = device.m_Buffers.Find(device.m_ConstantsBuffer);
		auto& constantsData = commandBuffer.m_PushConstantData[data.m_PushConstantsDataIndex];
		device.AssignBuffer(device.m_ConstantsBuffer, constantsData.data(), data.m_PushConstantsOffset, data.m_PushConstantsSize);
		glBindBufferBase(glBuffer->m_GLTarget, 0, glBuffer->m_GLBufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void OpenGLCommandImpl::DrawMesh_Impl(OpenGLDevice& device, const GL_CommandBuffer& commandBuffer, const GL_CommandData& data)
	{
		GL_Mesh* mesh = device.m_Meshes.Find(data.m_Mesh);
		if (!mesh)
		{
			device.Debug().LogError("OpenGLCommandImpl::DrawMesh: Invalid mesh handle.");
			return;
		}

		glBindVertexArray(mesh->m_GLVertexArrayID);
		OpenGLGraphicsModule::LogGLError(glGetError());

		++device.m_CurrentDrawCalls;
		device.m_CurrentVertices += mesh->m_VertexCount;
		if (mesh->m_IndexCount == 0) glDrawArrays(commandBuffer.m_GLCurrentPrimitives, 0, mesh->m_VertexCount);
		else
		{
			glDrawElements(commandBuffer.m_GLCurrentPrimitives, mesh->m_IndexCount, GL_UNSIGNED_INT, NULL);
			device.m_CurrentTriangles += mesh->m_IndexCount / 3;
		}
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindVertexArray(NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void OpenGLCommandImpl::Dispatch_Impl(OpenGLDevice& device, const GL_CommandBuffer&, const GL_CommandData& data)
	{
		glDispatchCompute((GLuint)data.m_XYZ.x, (GLuint)data.m_XYZ.y, (GLuint)data.m_XYZ.z);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void OpenGLCommandImpl::SetStencilTestEnabled_Impl(OpenGLDevice& device, const GL_CommandBuffer&, const GL_CommandData& data)
	{
		if (data.m_Enable)
			glEnable(GL_STENCIL_TEST);
		else
			glDisable(GL_STENCIL_TEST);
	}

	void OpenGLCommandImpl::SetStencilOp_Impl(OpenGLDevice& device, const GL_CommandBuffer&, const GL_CommandData& data)
	{
		const GLenum glCompareOp = CompareOps.at(CompareOp(data.m_CompareOp));
		const GLenum glFail = GLFuncs.at(Func(data.m_Fail));
		const GLenum glDepthFail = GLFuncs.at(Func(data.m_DepthFail));
		const GLenum glPass = GLFuncs.at(Func(data.m_Pass));
		glStencilOp(glFail, glDepthFail, glPass);
		glStencilFunc(glCompareOp, GLint(data.m_Reference), GLuint(data.m_Mask));
	}

	void OpenGLCommandImpl::SetStencilWriteMask_Impl(OpenGLDevice& device, const GL_CommandBuffer&, const GL_CommandData& data)
	{
		glStencilMask(GLuint(data.m_Mask));
	}

	void OpenGLCommandImpl::Commit_Impl(OpenGLDevice& device, const GL_CommandBuffer& commandBuffer)
	{
		for (size_t i = 0; i < commandBuffer.m_CommandsSize; ++i)
		{
			const GL_CommandData& data = commandBuffer.m_Commands[i];
			Command_Impl(device, commandBuffer, data);
		}
	}

	void OpenGLCommandImpl::SetViewport_Impl(OpenGLDevice& device, const GL_CommandBuffer&, const GL_CommandData& data)
	{
		glViewport(int(data.m_XYZFloat.x), int(data.m_XYZFloat.y), uint32_t(data.m_XYZFloat.z), uint32_t(data.m_XYZFloat.w));
	}

	void OpenGLCommandImpl::SetScissor_Impl(OpenGLDevice& device, const GL_CommandBuffer&, const GL_CommandData& data)
	{
		glEnable(GL_SCISSOR_TEST);
		glScissor(data.m_XYZSigned.x, data.m_XYZSigned.y, data.m_XYZSigned.z, data.m_XYZSigned.w);
	}

	void OpenGLCommandImpl::PipelineBarrier_Impl(OpenGLDevice& device, const GL_CommandBuffer&, const GL_CommandData& data)
	{
		glMemoryBarrier(data.m_FlagBits);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void OpenGLCommandImpl::CopyImage_Impl(OpenGLDevice& device, const GL_CommandBuffer&, const GL_CommandData& data)
	{
		GL_Texture* glSrcTexture = device.m_Textures.Find(data.m_SrcTexture);
		GL_Texture* glDstTexture = device.m_Textures.Find(data.m_DstTexture);
		if (!glSrcTexture)
		{
			device.Debug().LogError("OpenGLCommandImpl::CopyImage: Invalid src texture handle.");
			return;
		}
		if (!glDstTexture)
		{
			device.Debug().LogError("OpenGLCommandImpl::CopyImage: Invalid dst texture handle.");
			return;
		}

		glCopyImageSubData(glSrcTexture->m_GLTextureID, glSrcTexture->m_GLTextureType, 0, 0, 0, 0,
			glDstTexture->m_GLTextureID, glDstTexture->m_GLTextureType, 0, 0, 0, 0, glSrcTexture->m_Width, glSrcTexture->m_Height, 1);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void OpenGLCommandImpl::CopyImageToBuffer_Impl(OpenGLDevice& device, const GL_CommandBuffer&, const GL_CommandData& data)
	{
		GL_Texture* glSrcTexture = device.m_Textures.Find(data.m_SrcTexture);
		GL_Buffer* glDstBuffer = device.m_Buffers.Find(data.m_DstBuffer);
		if (!glSrcTexture)
		{
			device.Debug().LogError("OpenGLCommandImpl::CopyImageToBuffer: Invalid src texture handle.");
			return;
		}
		if (!glDstBuffer)
		{
			device.Debug().LogError("OpenGLCommandImpl::CopyImageToBuffer: Invalid dst buffer handle.");
			return;
		}

		glBindBuffer(GL_PIXEL_PACK_BUFFER, glDstBuffer->m_GLBufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindTexture(GL_TEXTURE_2D, glSrcTexture->m_GLTextureID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glGetTexImage(GL_TEXTURE_2D, 0, glSrcTexture->m_GLFormat, glSrcTexture->m_GLDataType, (void*)(0));
		OpenGLGraphicsModule::LogGLError(glGetError());

		glBindBuffer(GL_PIXEL_PACK_BUFFER, NULL);
		glBindTexture(GL_TEXTURE_2D, NULL);
	}
}