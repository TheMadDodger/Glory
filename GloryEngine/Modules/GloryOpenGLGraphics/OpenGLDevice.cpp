#include "OpenGLDevice.h"

#include "OpenGLGraphicsModule.h"
#include "GLShader.h"
#include "OGLMaterial.h"
#include "OGLPipeline.h"
#include "OGLRenderTexture.h"
#include "GLConverter.h"

#include <Engine.h>
#include <Debug.h>
#include <CubemapData.h>
#include <PipelineData.h>

namespace Glory
{
	OpenGLDevice::OpenGLDevice(OpenGLGraphicsModule* pModule): GraphicsDevice(pModule)
	{
	}

	OpenGLDevice::~OpenGLDevice()
	{
		m_Buffers.Clear();
		m_Meshes.Clear();
	}

	OpenGLGraphicsModule* OpenGLDevice::GraphicsModule()
	{
		return static_cast<OpenGLGraphicsModule*>(m_pModule);
	}

#pragma region Commands

	void OpenGLDevice::Begin()
	{
	}

	void OpenGLDevice::BeginRenderPass(RenderPassHandle handle)
	{
		GL_RenderPass* renderPass = m_RenderPasses.Find(handle);
		if (!renderPass)
		{
			Debug().LogError("OpenGLDevice::BeginRenderPass: Invalid render pass handle.");
			return;
		}
		GL_RenderTexture* renderTexture = m_RenderTextures.Find(renderPass->m_RenderTexture);
		if (!renderTexture)
		{
			Debug().LogError("OpenGLDevice::BeginRenderPass: Render pass has an invalid render texture handle.");
			return;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, renderTexture->m_GLFramebufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glViewport(0, 0, renderTexture->m_Width, renderTexture->m_Height);
	}

	void OpenGLDevice::BeginPipeline(PipelineHandle handle)
	{
		GL_Pipeline* pipeline = m_Pipelines.Find(handle);
		if (!pipeline)
		{
			Debug().LogError("OpenGLDevice::BeginPipeline: Invalid pipeline handle.");
			return;
		}

		glUseProgram(pipeline->m_GLProgramID);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void OpenGLDevice::End()
	{
	}

	void OpenGLDevice::EndRenderPass()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void OpenGLDevice::EndPipeline()
	{
		glUseProgram(NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void OpenGLDevice::BindBuffer(BufferHandle buffer)
	{
		GL_Buffer* glBuffer = m_Buffers.Find(buffer);
		if (!glBuffer)
		{
			Debug().LogError("OpenGLDevice::BindBuffer: Invalid buffer handle.");
			return;
		}

		glBindBuffer(glBuffer->m_GLTarget, glBuffer->m_GLBufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());

		const uint32_t bindingIndex = BindingIndex(glBuffer->m_Name);
		if (bindingIndex)
		{
			glBindBufferBase(glBuffer->m_GLTarget, (GLuint)bindingIndex, glBuffer->m_GLBufferID);
			OpenGLGraphicsModule::LogGLError(glGetError());
		}
	}

	void OpenGLDevice::BindDescriptorSets(PipelineHandle pipeline, std::vector<DescriptorSetHandle> sets)
	{
		for (size_t i = 0; i < sets.size(); ++i)
		{
			GL_DescriptorSet* glSet = m_Sets.Find(sets[i]);
			if (!glSet)
			{
				Debug().LogError("OpenGLDevice::BindDescriptorSets: Invalid set handle.");
				return;
			}

			for (size_t i = 0; i < glSet->m_Buffers.size(); ++i)
			{
				GL_Buffer* glBuffer = m_Buffers.Find(glSet->m_Buffers[i]);
				if (!glBuffer)
				{
					Debug().LogError("OpenGLDevice::BindDescriptorSet: Invalid buffer handle.");
					return;
				}

				glBindBuffer(glBuffer->m_GLTarget, glBuffer->m_GLBufferID);
				OpenGLGraphicsModule::LogGLError(glGetError());

				if (glSet->m_BindingIndices[i])
				{
					glBindBufferBase(glBuffer->m_GLTarget, (GLuint)glSet->m_BindingIndices[i], glBuffer->m_GLBufferID);
					OpenGLGraphicsModule::LogGLError(glGetError());
				}
			}
		}
	}

	void OpenGLDevice::DrawMesh(MeshHandle handle)
	{
		GL_Mesh* mesh = m_Meshes.Find(handle);
		if (!mesh)
		{
			Debug().LogError("OpenGLDevice::DrawMesh: Invalid mesh handle.");
			return;
		}

		glBindVertexArray(mesh->m_GLVertexArrayID);
		OpenGLGraphicsModule::LogGLError(glGetError());

		if (mesh->m_IndexCount == 0) glDrawArrays(mesh->m_GLPrimitiveType, 0, mesh->m_VertexCount);
		else glDrawElements(mesh->m_GLPrimitiveType, mesh->m_IndexCount, GL_UNSIGNED_INT, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindVertexArray(NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

#pragma endregion

#pragma region Resource Management

	BufferHandle OpenGLDevice::CreateBuffer(std::string&& name, size_t bufferSize, BufferType type)
	{
		BufferHandle handle;
		GL_Buffer& buffer = m_Buffers.Emplace(handle, GL_Buffer());
		buffer.m_Name = std::move(name);

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

		glBindBuffer(buffer.m_GLTarget, buffer.m_GLBufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBufferData(buffer.m_GLTarget, buffer.m_Size, NULL, buffer.m_GLUsage);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindBuffer(buffer.m_GLTarget, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());

		std::stringstream str;
		str << "OpenGLDevice: Buffer " << handle << " created with size " << bufferSize << ".";
		Debug().LogInfo(str.str());

		return handle;
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
			Debug().LogError("OpenGLDevice::AssignBuffer: Attempting to write beyong buffer size");
			return;
		}

		glBindBuffer(buffer->m_GLTarget, buffer->m_GLBufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBufferSubData(buffer->m_GLTarget, offset, size, data);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindBuffer(buffer->m_GLTarget, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	MeshHandle OpenGLDevice::CreateMesh(std::vector<BufferHandle>&& buffers, uint32_t vertexCount,
		uint32_t indexCount, uint32_t stride, PrimitiveType primitiveType,
		const std::vector<AttributeType>& attributeTypes)
	{
		MeshHandle handle;
		GL_Mesh& mesh = m_Meshes.Emplace(handle, GL_Mesh());
		mesh.m_Buffers = std::move(buffers);
		mesh.m_GLPrimitiveType = GLConverter::TO_GLPRIMITIVETYPE.at(primitiveType);
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

		std::stringstream str;
		str << "OpenGLDevice: Mesh " << handle << " created.";
		Debug().LogInfo(str.str());

		return handle;
	}

	TextureHandle OpenGLDevice::CreateTexture(TextureData* pTexture)
	{
		ImageData* pImageData = pTexture->GetImageData(&m_pModule->GetEngine()->GetAssetManager());
		if (!pImageData) return NULL;

		TextureHandle handle;
		GL_Texture& texture = m_Textures.Emplace(handle, GL_Texture());
		
		texture.m_GLTextureType = GL_TEXTURE_2D;

		if (pImageData->GetBytesPerPixel() == 1)
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glGenTextures(1, &texture.m_GLTextureID);
		OpenGLGraphicsModule::LogGLError(glGetError());

		glBindTexture(texture.m_GLTextureType, texture.m_GLTextureID);
		OpenGLGraphicsModule::LogGLError(glGetError());

		texture.m_GLFormat = GLConverter::TO_GLFORMAT.at(pImageData->GetFormat());
		texture.m_GLInternalFormat = pImageData->GetFormat() == PixelFormat::PF_Stencil ? GL_STENCIL_INDEX8 :
			GLConverter::TO_GLFORMAT.at(pImageData->GetInternalFormat());
		texture.m_GLDataType = GLConverter::TO_GLDATATYPE.at(pImageData->GetDataType());

		SamplerSettings& sampler = pTexture->GetSamplerSettings();
		texture.m_GLMinFilter = GLConverter::GetMinFilter(sampler.MipmapMode, sampler.MinFilter);
		texture.m_GLMagFilter = GLConverter::TO_GLFILTER.at(sampler.MagFilter);
		texture.m_GLTextureWrapS = GLConverter::TO_GLTEXTUREWRAP.at(sampler.AddressModeU);
		texture.m_GLTextureWrapT = GLConverter::TO_GLTEXTUREWRAP.at(sampler.AddressModeV);
		texture.m_GLTextureWrapR = GLConverter::TO_GLTEXTUREWRAP.at(sampler.AddressModeW);

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

		float aniso = 0.0f;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameterf(texture.m_GLTextureType, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
		OpenGLGraphicsModule::LogGLError(glGetError());

		glBindTexture(texture.m_GLTextureType, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());

		std::stringstream str;
		str << "OpenGLDevice: Texture " << handle << " created.";
		Debug().LogInfo(str.str());

		return handle;
	}

	TextureHandle OpenGLDevice::CreateTexture(const TextureCreateInfo& textureInfo, const void* pixels)
	{
		TextureHandle handle;
		GL_Texture& texture = m_Textures.Emplace(handle, GL_Texture());

		texture.m_GLTextureType = GLConverter::GetGLImageType(textureInfo.m_ImageType);

		texture.m_GLFormat = GLConverter::TO_GLFORMAT.at(textureInfo.m_PixelFormat);
		texture.m_GLInternalFormat = textureInfo.m_PixelFormat == PixelFormat::PF_Stencil ? GL_STENCIL_INDEX8 :
			GLConverter::TO_GLFORMAT.at(textureInfo.m_InternalFormat);
		texture.m_GLDataType = GLConverter::TO_GLDATATYPE.at(textureInfo.m_Type);

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

		texture.m_GLMinFilter = GLConverter::GetMinFilter(sampler.MipmapMode, sampler.MinFilter);
		texture.m_GLMagFilter = GLConverter::TO_GLFILTER.at(sampler.MagFilter);
		texture.m_GLTextureWrapS = GLConverter::TO_GLTEXTUREWRAP.at(sampler.AddressModeU);
		texture.m_GLTextureWrapT = GLConverter::TO_GLTEXTUREWRAP.at(sampler.AddressModeV);
		texture.m_GLTextureWrapR = GLConverter::TO_GLTEXTUREWRAP.at(sampler.AddressModeW);

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

		float aniso = 0.0f;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glTexParameterf(texture.m_GLTextureType, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
		OpenGLGraphicsModule::LogGLError(glGetError());

		glBindTexture(texture.m_GLTextureType, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());

		std::stringstream str;
		str << "OpenGLDevice: Texture " << handle << " created.";
		Debug().LogInfo(str.str());

		return handle;
	}

	RenderTextureHandle OpenGLDevice::CreateRenderTexture(RenderPassHandle renderPass, const RenderTextureCreateInfo& info)
	{
		if (info.Width == 0 || info.Height == 0)
		{
			Debug().LogError("OpenGLDevice::CreateRenderTexture: Invalid RenderTexture size.");
			return NULL;
		}

		GL_RenderPass* glRenderPass = m_RenderPasses.Find(renderPass);
		if (!glRenderPass)
		{
			Debug().LogError("OpenGLDevice::CreateRenderTexture: Invalid render pass handle");
			return NULL;
		}

		RenderTextureHandle handle;
		GL_RenderTexture& renderTexture = m_RenderTextures.Emplace(handle, GL_RenderTexture());
		renderTexture.m_RenderPass = renderPass;
		renderTexture.m_Width = info.Width;
		renderTexture.m_Height = info.Height;

		/* Create framebuffer */
		glGenFramebuffers(1, &renderTexture.m_GLFramebufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindFramebuffer(GL_FRAMEBUFFER, renderTexture.m_GLFramebufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());

		const size_t numAttachments = info.Attachments.size() + (info.HasDepth ? 1 : 0) + (info.HasStencil ? 1 : 0);
		renderTexture.m_AttachmentNames.resize(numAttachments);
		renderTexture.m_Textures.resize(numAttachments);

		SamplerSettings sampler;
		sampler.MipmapMode = Filter::F_None;
		sampler.MinFilter = Filter::F_Nearest;
		sampler.MagFilter = Filter::F_Nearest;

		size_t textureCounter = 0;
		for (size_t i = 0; i < info.Attachments.size(); ++i)
		{
			Attachment attachment = info.Attachments[i];
			renderTexture.m_Textures[i] = CreateTexture({info.Width, info.Height, attachment.Format, attachment.InternalFormat, attachment.ImageType, attachment.m_Type, 0, 0, attachment.ImageAspect, sampler});
			renderTexture.m_AttachmentNames[i] = attachment.Name;
			++textureCounter;
		}

		size_t depthIndex = 0, stencilIndex = 0;
		if (info.HasDepth)
		{
			depthIndex = textureCounter;
			renderTexture.m_Textures[depthIndex] = CreateTexture({ info.Width, info.Height, PixelFormat::PF_Depth, PixelFormat::PF_Depth32, ImageType::IT_2D, DataType::DT_UInt, 0, 0, ImageAspect::IA_Depth, sampler });
			renderTexture.m_AttachmentNames[depthIndex] = "Depth";
			++textureCounter;
		}

		if (info.HasStencil)
		{
			stencilIndex = textureCounter;
			renderTexture.m_Textures[stencilIndex] = CreateTexture({ info.Width, info.Height, PixelFormat::PF_Stencil, PixelFormat::PF_R8Uint, ImageType::IT_2D, DataType::DT_UInt, 0, 0, ImageAspect::IA_Stencil, sampler });
			renderTexture.m_AttachmentNames[stencilIndex] = "Stencil";
			++textureCounter;
		}

		// Initialize the framebuffer
		const size_t attachmentCount = info.Attachments.size();
		std::vector<GLenum> drawBuffers = std::vector<GLenum>(attachmentCount);
		for (uint32_t i = 0; i < attachmentCount; i++)
		{
			drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
			GL_Texture* glTexture = m_Textures.Find(renderTexture.m_Textures[i]);
			glFramebufferTexture(GL_FRAMEBUFFER, drawBuffers[i], glTexture->m_GLTextureID, 0);
			OpenGLGraphicsModule::LogGLError(glGetError());
		}

		if (info.HasDepth)
		{
			GL_Texture* glTexture = m_Textures.Find(renderTexture.m_Textures[depthIndex]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, glTexture->m_GLTextureID, 0);
			OpenGLGraphicsModule::LogGLError(glGetError());
		}
		if (info.HasStencil)
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
			return NULL;
		}

		glBindTexture(GL_TEXTURE_2D, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindFramebuffer(GL_FRAMEBUFFER, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());

		std::stringstream str;
		str << "OpenGLDevice: RenderTexture " << handle << " created with " << renderTexture.m_Textures.size() << " attachments.";
		Debug().LogInfo(str.str());

		return handle;
	}

	RenderPassHandle OpenGLDevice::CreateRenderPass(const RenderPassInfo& info)
	{
		if (info.RenderTextureInfo.Width == 0 || info.RenderTextureInfo.Height == 0)
		{
			Debug().LogError("OpenGLDevice::CreateRenderPass: Invalid RenderTexture size.");
			return NULL;
		}

		RenderPassHandle handle;
		GL_RenderPass& renderPass = m_RenderPasses.Emplace(handle, GL_RenderPass());
		renderPass.m_RenderTexture = CreateRenderTexture(handle, info.RenderTextureInfo);

		if (renderPass.m_RenderTexture == NULL)
		{
			m_RenderPasses.Erase(handle);
			Debug().LogError("OpenGLDevice::CreateRenderPass: Failed to create RenderTexture for RenderPass.");
			return NULL;
		}

		std::stringstream str;
		str << "OpenGLDevice: RenderPass " << handle << " created.";
		Debug().LogInfo(str.str());

		return handle;
	}

	ShaderHandle OpenGLDevice::CreateShader(const FileData* pShaderFileData, const ShaderType& shaderType, const std::string& function)
	{
		ShaderHandle handle;
		GL_Shader& shader = m_Shaders.Emplace(handle, GL_Shader());
		
		const char* shaderSource = pShaderFileData->Data();

		shader.m_GLShaderType = GLConverter::GetShaderStageFlag(shaderType);
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

		std::stringstream str;
		str << "OpenGLDevice: Shader " << handle << " created.";
		Debug().LogInfo(str.str());

		return handle;
	}

	PipelineHandle OpenGLDevice::CreatePipeline(RenderPassHandle renderPass, PipelineData* pPipeline, std::vector<DescriptorSetHandle>&&, size_t, const std::vector<AttributeType>&)
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

		int success;
		char infoLog[512];

		pipeline.m_GLProgramID = glCreateProgram();
		OpenGLGraphicsModule::LogGLError(glGetError());

		for (size_t i = 0; i < pPipeline->ShaderCount(); ++i)
		{
			const FileData* pShader = pPipeline->Shader(pipelines, i);
			const ShaderType type = pPipeline->GetShaderType(pipelines, i);
			ShaderHandle shaderHandle = CreateShader(pShader, type, "main");
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

		std::stringstream str;
		str << "OpenGLDevice: Pipeline " << handle << " created.";
		Debug().LogInfo(str.str());

		return handle;
	}

	DescriptorSetHandle OpenGLDevice::CreateDescriptorSet(std::vector<BufferHandle>&& bufferHandles)
	{
		std::vector<uint32_t> bindingIndices;
		bindingIndices.resize(bufferHandles.size());

		for (size_t i = 0; i < bufferHandles.size(); ++i)
		{
			GL_Buffer* glBuffer = m_Buffers.Find(bufferHandles[i]);
			if (!glBuffer)
			{
				Debug().LogError("OpenGLDevice::CreateDescriptorSet: Invalid buffer handle.");
				return NULL;
			}

			bindingIndices[i] = BindingIndex(glBuffer->m_Name);
		}

		DescriptorSetHandle handle;
		GL_DescriptorSet& set = m_Sets.Emplace(handle, GL_DescriptorSet());
		set.m_Buffers = std::move(bufferHandles);
		set.m_BindingIndices = std::move(bindingIndices);

		std::stringstream str;
		str << "OpenGLDevice: Descriptor set " << handle << " created.";
		Debug().LogInfo(str.str());

		return handle;
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

		std::stringstream str;
		str << "OpenGLDevice: Buffer " << handle << " was freed from device memory.";
		Debug().LogInfo(str.str());

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

		std::stringstream str;
		str << "OpenGLDevice: Mesh " << handle << " was freed from device memory.";
		Debug().LogInfo(str.str());

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

		std::stringstream str;
		str << "OpenGLDevice: Texture " << handle << " was freed from device memory.";
		Debug().LogInfo(str.str());

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

		std::stringstream str;
		str << "OpenGLDevice: RenderTexture " << handle << " was freed from device memory.";
		Debug().LogInfo(str.str());

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

		std::stringstream str;
		str << "OpenGLDevice: RenderPass " << handle << " was freed from device memory.";
		Debug().LogInfo(str.str());

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

		std::stringstream str;
		str << "OpenGLDevice: Shader " << handle << " was freed from device memory.";
		Debug().LogInfo(str.str());

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

		std::stringstream str;
		str << "OpenGLDevice: Pipeline " << handle << " was freed from device memory.";
		Debug().LogInfo(str.str());

		handle = 0;
	}

#pragma endregion
}
