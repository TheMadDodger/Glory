#include "OpenGLDevice.h"

#include "OpenGLGraphicsModule.h"

#include <Engine.h>
#include <Debug.h>

#include <PipelineData.h>
#include <ImageData.h>
#include <TextureData.h>
#include <FileData.h>

namespace Glory
{
	const std::map<PixelFormat, GLuint> Formats = {
		{ PixelFormat::PF_Undefined,                 0 },

		// Basic format
		{ PixelFormat::PF_R,                         GL_RED },
		{ PixelFormat::PF_RG,                        GL_RG },
		{ PixelFormat::PF_RGB,                       GL_RGB },
		{ PixelFormat::PF_BGR,                       GL_BGR },
		{ PixelFormat::PF_RGBA,                      GL_RGBA },
		{ PixelFormat::PF_BGRA,                      GL_BGRA },
		{ PixelFormat::PF_RI,                        GL_RED_INTEGER },
		{ PixelFormat::PF_RGI,                       GL_RG_INTEGER },
		{ PixelFormat::PF_RGBI,                      GL_RGB_INTEGER },
		{ PixelFormat::PF_BGRI,                      GL_BGR_INTEGER },
		{ PixelFormat::PF_RGBAI,                     GL_RGBA_INTEGER },
		{ PixelFormat::PF_BGRAI,                     GL_BGRA_INTEGER },
		{ PixelFormat::PF_Stencil,                   GL_STENCIL_INDEX },
		{ PixelFormat::PF_Depth,                     GL_DEPTH_COMPONENT },
		{ PixelFormat::PF_DepthStencil,              GL_DEPTH_STENCIL },

		// Internal format
		{ PixelFormat::PF_R4G4UnormPack8,            0 },
		{ PixelFormat::PF_R4G4B4A4UnormPack16,       0 },
		{ PixelFormat::PF_B4G4R4A4UnormPack16,       0 },
		{ PixelFormat::PF_R5G6B5UnormPack16,         0 },
		{ PixelFormat::PF_B5G6R5UnormPack16,         0 },
		{ PixelFormat::PF_R5G5B5A1UnormPack16,       0 },
		{ PixelFormat::PF_B5G5R5A1UnormPack16,       0 },
		{ PixelFormat::PF_A1R5G5B5UnormPack16,       0 },
		{ PixelFormat::PF_R8Unorm,                   0 },
		{ PixelFormat::PF_R8Snorm,                   0 },
		{ PixelFormat::PF_R8Uscaled,                 0 },
		{ PixelFormat::PF_R8Sscaled,                 0 },
		{ PixelFormat::PF_R8Uint,                    GL_R8UI },
		{ PixelFormat::PF_R8Sint,                    GL_R8I },
		{ PixelFormat::PF_R8Srgb,                    GL_R8 },
		{ PixelFormat::PF_R8G8Unorm,                 0 },
		{ PixelFormat::PF_R8G8Snorm,                 0 },
		{ PixelFormat::PF_R8G8Uscaled,               0 },
		{ PixelFormat::PF_R8G8Sscaled,               0 },
		{ PixelFormat::PF_R8G8Uint,                  0 },
		{ PixelFormat::PF_R8G8Sint,                  0 },
		{ PixelFormat::PF_R8G8Srgb,                  GL_RG8 },
		{ PixelFormat::PF_R8G8B8Unorm,               GL_RGB8_SNORM },
		{ PixelFormat::PF_R8G8B8Snorm,               GL_RGB8_SNORM },
		{ PixelFormat::PF_R8G8B8Uscaled,             0 },
		{ PixelFormat::PF_R8G8B8Sscaled,             0 },
		{ PixelFormat::PF_R8G8B8Uint,                0 },
		{ PixelFormat::PF_R8G8B8Sint,                0 },
		{ PixelFormat::PF_R8G8B8Srgb,                GL_RGB8 },
		{ PixelFormat::PF_B8G8R8Unorm,               0 },
		{ PixelFormat::PF_B8G8R8Snorm,               0 },
		{ PixelFormat::PF_B8G8R8Uscaled,             0 },
		{ PixelFormat::PF_B8G8R8Sscaled,             0 },
		{ PixelFormat::PF_B8G8R8Uint,                0 },
		{ PixelFormat::PF_B8G8R8Sint,                0 },
		{ PixelFormat::PF_B8G8R8Srgb,                0 },
		{ PixelFormat::PF_R8G8B8A8Unorm,             GL_RGBA8_SNORM },
		{ PixelFormat::PF_R8G8B8A8Snorm,             GL_RGBA8_SNORM },
		{ PixelFormat::PF_R8G8B8A8Uscaled,           0 },
		{ PixelFormat::PF_R8G8B8A8Sscaled,           0 },
		{ PixelFormat::PF_R8G8B8A8Uint,              0 },
		{ PixelFormat::PF_R8G8B8A8Sint,              0 },
		{ PixelFormat::PF_R8G8B8A8Srgb,              GL_RGBA8 },
		{ PixelFormat::PF_B8G8R8A8Unorm,             0 },
		{ PixelFormat::PF_B8G8R8A8Snorm,             0 },
		{ PixelFormat::PF_B8G8R8A8Uscaled,           0 },
		{ PixelFormat::PF_B8G8R8A8Sscaled,           0 },
		{ PixelFormat::PF_B8G8R8A8Uint,              0 },
		{ PixelFormat::PF_B8G8R8A8Sint,              0 },
		{ PixelFormat::PF_B8G8R8A8Srgb,              0 },
		{ PixelFormat::PF_A8B8G8R8UnormPack32,       0 },
		{ PixelFormat::PF_A8B8G8R8SnormPack32,       0 },
		{ PixelFormat::PF_A8B8G8R8UscaledPack32,     0 },
		{ PixelFormat::PF_A8B8G8R8SscaledPack32,     0 },
		{ PixelFormat::PF_A8B8G8R8UintPack32,        0 },
		{ PixelFormat::PF_A8B8G8R8SintPack32,        0 },
		{ PixelFormat::PF_A8B8G8R8SrgbPack32,        0 },
		{ PixelFormat::PF_A2R10G10B10UnormPack32,    0 },
		{ PixelFormat::PF_A2R10G10B10SnormPack32,    0 },
		{ PixelFormat::PF_A2R10G10B10UscaledPack32,  0 },
		{ PixelFormat::PF_A2R10G10B10SscaledPack32,  0 },
		{ PixelFormat::PF_A2R10G10B10UintPack32,     0 },
		{ PixelFormat::PF_A2R10G10B10SintPack32,     0 },
		{ PixelFormat::PF_A2B10G10R10UnormPack32,    0 },
		{ PixelFormat::PF_A2B10G10R10SnormPack32,    0 },
		{ PixelFormat::PF_A2B10G10R10UscaledPack32,  0 },
		{ PixelFormat::PF_A2B10G10R10SscaledPack32,  0 },
		{ PixelFormat::PF_A2B10G10R10UintPack32,     0 },
		{ PixelFormat::PF_A2B10G10R10SintPack32,     0 },
		{ PixelFormat::PF_R16Unorm,                  0 },
		{ PixelFormat::PF_R16Snorm,                  0 },
		{ PixelFormat::PF_R16Uscaled,                0 },
		{ PixelFormat::PF_R16Sscaled,                0 },
		{ PixelFormat::PF_R16Uint,                   GL_R16UI },
		{ PixelFormat::PF_R16Sint,                   GL_R16I },
		{ PixelFormat::PF_R16Sfloat,                 0 },
		{ PixelFormat::PF_R16G16Unorm,               0 },
		{ PixelFormat::PF_R16G16Snorm,               0 },
		{ PixelFormat::PF_R16G16Uscaled,             0 },
		{ PixelFormat::PF_R16G16Sscaled,             0 },
		{ PixelFormat::PF_R16G16Uint,                0 },
		{ PixelFormat::PF_R16G16Sint,                0 },
		{ PixelFormat::PF_R16G16Sfloat,              0 },
		{ PixelFormat::PF_R16G16B16Unorm,            0 },
		{ PixelFormat::PF_R16G16B16Snorm,            0 },
		{ PixelFormat::PF_R16G16B16Uscaled,          0 },
		{ PixelFormat::PF_R16G16B16Sscaled,          0 },
		{ PixelFormat::PF_R16G16B16Uint,             0 },
		{ PixelFormat::PF_R16G16B16Sint,             0 },
		{ PixelFormat::PF_R16G16B16Sfloat,           GL_RGB16F },
		{ PixelFormat::PF_R16G16B16A16Unorm,         0 },
		{ PixelFormat::PF_R16G16B16A16Snorm ,        0 },
		{ PixelFormat::PF_R16G16B16A16Uscaled,       0 },
		{ PixelFormat::PF_R16G16B16A16Sscaled,       0 },
		{ PixelFormat::PF_R16G16B16A16Uint,          0 },
		{ PixelFormat::PF_R16G16B16A16Sint,          0 },
		{ PixelFormat::PF_R16G16B16A16Sfloat,        GL_RGBA16F },
		{ PixelFormat::PF_R32Uint,                   GL_R32UI },
		{ PixelFormat::PF_R32Sint,                   GL_R32I },
		{ PixelFormat::PF_R32Sfloat,                 GL_R32F },
		{ PixelFormat::PF_R32G32Uint,                0 },
		{ PixelFormat::PF_R32G32Sint,                0 },
		{ PixelFormat::PF_R32G32Sfloat,              0 },
		{ PixelFormat::PF_R32G32B32Uint,             0 },
		{ PixelFormat::PF_R32G32B32Sint,             0 },
		{ PixelFormat::PF_R32G32B32Sfloat,           0 },
		{ PixelFormat::PF_R32G32B32A32Uint,          GL_RGBA32UI },
		{ PixelFormat::PF_R32G32B32A32Sint,          0 },
		{ PixelFormat::PF_R32G32B32A32Sfloat,        0 },
		{ PixelFormat::PF_R64Uint,                   0 }, // Not supported
		{ PixelFormat::PF_R64Sint,                   0 }, // Not supported
		{ PixelFormat::PF_R64Sfloat,                 0 },
		{ PixelFormat::PF_R64G64Uint,                0 },
		{ PixelFormat::PF_R64G64Sint,                0 },
		{ PixelFormat::PF_R64G64Sfloat,              0 },
		{ PixelFormat::PF_R64G64B64Uint,             0 },
		{ PixelFormat::PF_R64G64B64Sint,             0 },
		{ PixelFormat::PF_R64G64B64Sfloat,           0 },
		{ PixelFormat::PF_R64G64B64A64Uint,          0 },
		{ PixelFormat::PF_R64G64B64A64Sint,          0 },
		{ PixelFormat::PF_R64G64B64A64Sfloat,        0 },
		{ PixelFormat::PF_B10G11R11UfloatPack32,     0 },
		{ PixelFormat::PF_E5B9G9R9UfloatPack32,      0 },
		{ PixelFormat::PF_D16Unorm,                  0 },
		{ PixelFormat::PF_X8D24UnormPack32,          0 },
		{ PixelFormat::PF_D32Sfloat,                 0 },
		{ PixelFormat::PF_S8Uint,                    0 },
		{ PixelFormat::PF_D16UnormS8Uint,            0 },
		{ PixelFormat::PF_D24UnormS8Uint,            0 },
		{ PixelFormat::PF_D32SfloatS8Uint,           0 },
		{ PixelFormat::PF_Bc1RgbUnormBlock,          0 },
		{ PixelFormat::PF_Bc1RgbSrgbBlock,           0 },
		{ PixelFormat::PF_Bc1RgbaUnormBlock,         0 },
		{ PixelFormat::PF_Bc1RgbaSrgbBlock,          0 },
		{ PixelFormat::PF_Bc2UnormBlock,             0 },
		{ PixelFormat::PF_Bc2SrgbBlock,              0 },
		{ PixelFormat::PF_Bc3UnormBlock,             0 },
		{ PixelFormat::PF_Bc3SrgbBlock,              0 },
		{ PixelFormat::PF_Bc4UnormBlock,             0 },
		{ PixelFormat::PF_Bc4SnormBlock,             0 },
		{ PixelFormat::PF_Bc5UnormBlock,             0 },
		{ PixelFormat::PF_Bc5SnormBlock,             0 },
		{ PixelFormat::PF_Bc6HUfloatBlock,           0 },
		{ PixelFormat::PF_Bc6HSfloatBlock,           0 },
		{ PixelFormat::PF_Bc7UnormBlock,             0 },
		{ PixelFormat::PF_Bc7SrgbBlock,              0 },
		{ PixelFormat::PF_Etc2R8G8B8UnormBlock,      0 },
		{ PixelFormat::PF_Etc2R8G8B8SrgbBlock,       0 },
		{ PixelFormat::PF_Etc2R8G8B8A1UnormBlock,    0 },
		{ PixelFormat::PF_Etc2R8G8B8A1SrgbBlock,     0 },
		{ PixelFormat::PF_Etc2R8G8B8A8UnormBlock,    0 },
		{ PixelFormat::PF_Etc2R8G8B8A8SrgbBlock,     0 },
		{ PixelFormat::PF_EacR11UnormBlock,          0 },
		{ PixelFormat::PF_EacR11SnormBlock,          0 },
		{ PixelFormat::PF_EacR11G11UnormBlock,       0 },
		{ PixelFormat::PF_EacR11G11SnormBlock,       0 },
		{ PixelFormat::PF_Astc4x4UnormBlock,         0 },
		{ PixelFormat::PF_Astc4x4SrgbBlock,          0 },
		{ PixelFormat::PF_Astc5x4UnormBlock,         0 },
		{ PixelFormat::PF_Astc5x4SrgbBlock,          0 },
		{ PixelFormat::PF_Astc5x5UnormBlock,         0 },
		{ PixelFormat::PF_Astc5x5SrgbBlock,          0 },
		{ PixelFormat::PF_Astc6x5UnormBlock,         0 },
		{ PixelFormat::PF_Astc6x5SrgbBlock,          0 },
		{ PixelFormat::PF_Astc6x6UnormBlock,         0 },
		{ PixelFormat::PF_Astc6x6SrgbBlock,          0 },
		{ PixelFormat::PF_Astc8x5UnormBlock,         0 },
		{ PixelFormat::PF_Astc8x5SrgbBlock,          0 },
		{ PixelFormat::PF_Astc8x6UnormBlock,         0 },
		{ PixelFormat::PF_Astc8x6SrgbBlock,          0 },
		{ PixelFormat::PF_Astc8x8UnormBlock,         0 },
		{ PixelFormat::PF_Astc8x8SrgbBlock,          0 },
		{ PixelFormat::PF_Astc10x5UnormBlock,        0 },
		{ PixelFormat::PF_Astc10x5SrgbBlock,         0 },
		{ PixelFormat::PF_Astc10x6UnormBlock,        0 },
		{ PixelFormat::PF_Astc10x6SrgbBlock,         0 },
		{ PixelFormat::PF_Astc10x8UnormBlock,        0 },
		{ PixelFormat::PF_Astc10x8SrgbBlock,         0 },
		{ PixelFormat::PF_Astc10x10UnormBlock,       0 },
		{ PixelFormat::PF_Astc10x10SrgbBlock,        0 },
		{ PixelFormat::PF_Astc12x10UnormBlock,       0 },
		{ PixelFormat::PF_Astc12x10SrgbBlock,        0 },
		{ PixelFormat::PF_Astc12x12UnormBlock,       0 },
		{ PixelFormat::PF_Astc12x12SrgbBlock,        0 },
		{ PixelFormat::PF_Depth16,                   GL_DEPTH_COMPONENT16 },
		{ PixelFormat::PF_Depth24,                   GL_DEPTH_COMPONENT24 },
		{ PixelFormat::PF_Depth32,                   GL_DEPTH_COMPONENT32 },
	};

	const std::map<PrimitiveType, GLuint> PrimitiveTypes =
	{
		{ PrimitiveType::PT_Point, GL_POINT  },
		{ PrimitiveType::PT_LineStrip, GL_LINE_STRIP  },
		{ PrimitiveType::PT_LineLoop, GL_LINE_LOOP  },
		{ PrimitiveType::PT_Lines, GL_LINES  },
		{ PrimitiveType::PT_LineStripAdjacency, GL_LINE_STRIP_ADJACENCY  },
		{ PrimitiveType::PT_LinesAdjacency, GL_LINES_ADJACENCY  },
		{ PrimitiveType::PT_TriangleStrip, GL_TRIANGLE_STRIP },
		{ PrimitiveType::PT_TriangleFan, GL_TRIANGLE_FAN },
		{ PrimitiveType::PT_Triangles, GL_TRIANGLES },
		{ PrimitiveType::PT_TriangleStripAdjacency, GL_TRIANGLE_STRIP_ADJACENCY },
		{ PrimitiveType::PT_TrianglesAdjacency, GL_TRIANGLES_ADJACENCY },
		{ PrimitiveType::PT_Patches, GL_PATCHES },
	};

	const std::map<Filter, GLint> Filters = {
		{ Filter::F_Linear, GL_LINEAR },
		{ Filter::F_Nearest, GL_NEAREST },
		{ Filter::F_CubicEXT, GL_CUBIC_EXT },
		{ Filter::F_CubicIMG, GL_CUBIC_IMG },
	};

	const std::map<SamplerAddressMode, GLint> Texturewraps = {
		{ SamplerAddressMode::SAM_Repeat, GL_REPEAT },
		{ SamplerAddressMode::SAM_ClampToEdge, GL_CLAMP_TO_EDGE },
		{ SamplerAddressMode::SAM_ClampToBorder, GL_CLAMP_TO_BORDER },
		{ SamplerAddressMode::SAM_MirroredRepeat, GL_MIRRORED_REPEAT },
		{ SamplerAddressMode::SAM_MirrorClampToEdge, GL_MIRROR_CLAMP_TO_EDGE },
		{ SamplerAddressMode::SAM_MirrorClampToEdgeKHR, GL_MIRROR_CLAMP_TO_EDGE_EXT },
	};

	const std::map<DataType, GLenum> Datatypes = {
		{ DataType::DT_Byte, GL_BYTE },
		{ DataType::DT_UByte, GL_UNSIGNED_BYTE },
		{ DataType::DT_Short, GL_SHORT },
		{ DataType::DT_UShort, GL_UNSIGNED_SHORT },
		{ DataType::DT_Int, GL_INT },
		{ DataType::DT_UInt, GL_UNSIGNED_INT },
		{ DataType::DT_Float, GL_FLOAT },
		{ DataType::DT_2Bytes, GL_2_BYTES },
		{ DataType::DT_3Bytes, GL_3_BYTES },
		{ DataType::DT_4Bytes, GL_4_BYTES },
		{ DataType::DT_Double, GL_DOUBLE },
	};

	GLint GetMinFilter(Filter mipMap, Filter minFilter)
	{
		switch (mipMap)
		{
		case Glory::Filter::F_None:
			return Filters.at(minFilter);
		case Glory::Filter::F_Nearest:
			switch (minFilter)
			{
			case Glory::Filter::F_Nearest:
				return GL_NEAREST_MIPMAP_NEAREST;
			case Glory::Filter::F_Linear:
				return GL_LINEAR_MIPMAP_NEAREST;
			}
			break;
		case Glory::Filter::F_Linear:
			switch (minFilter)
			{
			case Glory::Filter::F_Nearest:
				return GL_NEAREST_MIPMAP_LINEAR;
			case Glory::Filter::F_Linear:
				return GL_LINEAR_MIPMAP_LINEAR;
			}
			break;
		}
		return Filters.at(minFilter);
	}

	GLuint GetShaderStageFlag(ShaderType shaderType)
	{
		switch (shaderType)
		{
		case ShaderType::ST_Vertex:
			return GL_VERTEX_SHADER;
		case ShaderType::ST_Fragment:
			return GL_FRAGMENT_SHADER;
		case ShaderType::ST_Geomtery:
			return GL_GEOMETRY_SHADER;
		case ShaderType::ST_TessControl:
			return GL_TESS_CONTROL_SHADER;
		case ShaderType::ST_TessEval:
			return GL_TESS_EVALUATION_SHADER;
		case ShaderType::ST_Compute:
			return GL_COMPUTE_SHADER;
		}

		return 0;
	}

	GLuint GetGLImageType(ImageType imageType)
	{
		switch (imageType)
		{
		case Glory::ImageType::IT_UNDEFINED:
			break;
		case Glory::ImageType::IT_1D:
			return GL_TEXTURE_1D;
		case Glory::ImageType::IT_2D:
			return GL_TEXTURE_2D;
		case Glory::ImageType::IT_3D:
			return GL_TEXTURE_3D;
		case Glory::ImageType::IT_Cube:
			return GL_TEXTURE_CUBE_MAP;
		case Glory::ImageType::IT_1DArray:
			return GL_TEXTURE_1D_ARRAY;
		case Glory::ImageType::IT_2DArray:
			return GL_TEXTURE_2D_ARRAY;
		case Glory::ImageType::IT_CubeArray:
			return GL_TEXTURE_CUBE_MAP_ARRAY;
		default:
			break;
		}

		return 0;
	}

	OpenGLDevice::OpenGLDevice(OpenGLGraphicsModule* pModule): GraphicsDevice(pModule)
	{
		m_APIFeatures = APIFeatures::All & ~APIFeatures::PushConstants;
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

	CommandBufferHandle OpenGLDevice::Begin()
	{
		return CommandBufferHandle();
	}

	void OpenGLDevice::BeginRenderPass(CommandBufferHandle, RenderPassHandle renderPass)
	{
		GL_RenderPass* glRenderPass = m_RenderPasses.Find(renderPass);
		if (!glRenderPass)
		{
			Debug().LogError("OpenGLDevice::BeginRenderPass: Invalid render pass handle.");
			return;
		}
		GL_RenderTexture* glRenderTexture = m_RenderTextures.Find(glRenderPass->m_RenderTexture);
		if (!glRenderTexture)
		{
			Debug().LogError("OpenGLDevice::BeginRenderPass: Render pass has an invalid render texture handle.");
			return;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, glRenderTexture->m_GLFramebufferID);
		
		const bool hasDepth = glRenderTexture->m_Info.HasDepth;
		const bool hasStencil = glRenderTexture->m_Info.HasStencil;
		const bool hasStencilOrDepth = hasDepth || hasStencil;
		const bool hasColor = glRenderTexture->m_Textures.size() > hasStencilOrDepth ? 1 : 0;

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
		glClear(clearFlags);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glViewport(0, 0, glRenderTexture->m_Info.Width, glRenderTexture->m_Info.Height);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void OpenGLDevice::BeginPipeline(CommandBufferHandle, PipelineHandle pipeline)
	{
		GL_Pipeline* glPipeline = m_Pipelines.Find(pipeline);
		if (!glPipeline)
		{
			Debug().LogError("OpenGLDevice::BeginPipeline: Invalid pipeline handle.");
			return;
		}

		glUseProgram(glPipeline->m_GLProgramID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glPipeline->m_TextureCounter = 0;
	}

	void OpenGLDevice::End(CommandBufferHandle)
	{
	}

	void OpenGLDevice::EndRenderPass(CommandBufferHandle)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void OpenGLDevice::EndPipeline(CommandBufferHandle)
	{
		glUseProgram(NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void OpenGLDevice::BindDescriptorSets(CommandBufferHandle, PipelineHandle pipeline, std::vector<DescriptorSetHandle> sets, uint32_t)
	{
		GL_Pipeline* glPipeline = m_Pipelines.Find(pipeline);
		if (!glPipeline)
		{
			Debug().LogError("OpenGLDevice::BindDescriptorSet: Invalid pipeline handle.");
			return;
		}

		for (size_t i = 0; i < sets.size(); ++i)
		{
			GL_DescriptorSet* glSet = m_Sets.Find(sets[i]);
			if (!glSet)
			{
				Debug().LogError("OpenGLDevice::BindDescriptorSets: Invalid set handle.");
				return;
			}

			GL_DescriptorSetLayout* glSetLayout = m_SetLayouts.Find(glSet->m_Layout);
			if (!glSetLayout)
			{
				Debug().LogError("OpenGLDevice::BindDescriptorSets: Invalid set layout handle.");
				return;
			}

			size_t index = 0;
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
				glBindBufferBase(glBuffer->m_GLTarget, (GLuint)glSetLayout->m_BindingIndices[index], glBuffer->m_GLBufferID);
				OpenGLGraphicsModule::LogGLError(glGetError());
				++index;
			}

			for (size_t i = 0; i < glSet->m_Textures.size(); ++i)
			{
				GL_Texture* glTexture = m_Textures.Find(glSet->m_Textures[i]);

				GLuint texLocation = glGetUniformLocation(glPipeline->m_GLProgramID, glSetLayout->m_SamplerNames[i].c_str());
				OpenGLGraphicsModule::LogGLError(glGetError());
				glUniform1i(texLocation, glPipeline->m_TextureCounter);
				OpenGLGraphicsModule::LogGLError(glGetError());

				glActiveTexture(GL_TEXTURE0 + glPipeline->m_TextureCounter);
				OpenGLGraphicsModule::LogGLError(glGetError());
				glBindTexture(GL_TEXTURE_2D, glTexture ? glTexture->m_GLTextureID : 0);
				OpenGLGraphicsModule::LogGLError(glGetError());

				glActiveTexture(GL_TEXTURE0);
				OpenGLGraphicsModule::LogGLError(glGetError());
				++glPipeline->m_TextureCounter;
			}
		}
	}

	void OpenGLDevice::PushConstants(CommandBufferHandle, PipelineHandle, uint32_t, uint32_t, const void*, ShaderTypeFlag)
	{
		Debug().LogError("OpenGLDevice::PushConstants: Not supported on OpenGL device.");
	}

	void OpenGLDevice::DrawMesh(CommandBufferHandle, MeshHandle handle)
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

	void OpenGLDevice::Dispatch(CommandBufferHandle, uint32_t x, uint32_t y, uint32_t z)
	{
		glDispatchCompute((GLuint)x, (GLuint)y, (GLuint)z);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void OpenGLDevice::Commit(CommandBufferHandle)
	{
		glFlush();
	}

	void OpenGLDevice::Wait(CommandBufferHandle)
	{
	}

	void OpenGLDevice::Release(CommandBufferHandle)
	{
	}

	void OpenGLDevice::SetViewport(CommandBufferHandle, float x, float y, float width, float height, float minDepth, float maxDepth)
	{
		glViewport(int(x), int(y), uint32_t(width), uint32_t(height));
	}

	void OpenGLDevice::SetScissor(CommandBufferHandle, int x, int y, uint32_t width, uint32_t height)
	{
		glScissor(x, y, width, height);
	}

	void OpenGLDevice::PipelineBarrier(CommandBufferHandle commandBuffer, std::vector<BufferHandle>,
		std::vector<TextureHandle>, PipelineStageFlagBits, PipelineStageFlagBits)
	{
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

#pragma endregion

#pragma region Resource Management

	BufferHandle OpenGLDevice::CreateBuffer(size_t bufferSize, BufferType type)
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
		mesh.m_GLPrimitiveType = PrimitiveTypes.at(primitiveType);
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
		glTexParameterf(texture.m_GLTextureType, GL_TEXTURE_LOD_BIAS, 0.0);
		OpenGLGraphicsModule::LogGLError(glGetError());

		float aniso = 0.0f;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
		OpenGLGraphicsModule::LogGLError(glGetError());
		aniso = std::min(sampler.MaxAnisotropy, aniso);
		glTexParameterf(texture.m_GLTextureType, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
		OpenGLGraphicsModule::LogGLError(glGetError());

		glBindTexture(texture.m_GLTextureType, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());

		std::stringstream str;
		str << "OpenGLDevice: Texture " << handle << " created.";
		Debug().LogInfo(str.str());

		return handle;
	}

	TextureHandle OpenGLDevice::CreateTexture(const TextureCreateInfo& textureInfo, const void* pixels, size_t)
	{
		TextureHandle handle;
		GL_Texture& texture = m_Textures.Emplace(handle, GL_Texture());

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
		glTexParameterf(texture.m_GLTextureType, GL_TEXTURE_LOD_BIAS, 0.0);
		OpenGLGraphicsModule::LogGLError(glGetError());

		float aniso = 0.0f;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
		OpenGLGraphicsModule::LogGLError(glGetError());
		aniso = std::min(sampler.MaxAnisotropy, aniso);
		glTexParameterf(texture.m_GLTextureType, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
		OpenGLGraphicsModule::LogGLError(glGetError());

		glBindTexture(texture.m_GLTextureType, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());

		std::stringstream str;
		str << "OpenGLDevice: Texture " << handle << " created.";
		Debug().LogInfo(str.str());

		return handle;
	}

	RenderTextureHandle OpenGLDevice::CreateRenderTexture(RenderPassHandle renderPass, RenderTextureCreateInfo&& info)
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
		renderTexture.m_Info = std::move(info);
		CreateRenderTexture(renderTexture);

		std::stringstream str;
		str << "OpenGLDevice: RenderTexture " << handle << " created with " << renderTexture.m_Textures.size() << " attachments.";
		Debug().LogInfo(str.str());

		return handle;
	}

	TextureHandle OpenGLDevice::GetRenderTextureAttachment(RenderTextureHandle renderTexture, size_t index)
	{
		GL_RenderTexture* glRenderTexture = m_RenderTextures.Find(renderTexture);
		if (!glRenderTexture)
		{
			Debug().LogError("VulkanDevice::GetRenderTextureAttatchment: Invalid render texture handle");
			return NULL;
		}

		if (index >= glRenderTexture->m_Textures.size())
		{
			Debug().LogError("VulkanDevice::GetRenderTextureAttatchment: Invalid attachment index");
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
		renderPass.m_RenderTexture = CreateRenderTexture(handle, std::move(info.RenderTextureInfo));
		renderPass.m_ClearColor = std::move(info.m_ClearColor);
		renderPass.m_DepthClear = info.m_DepthClear;
		renderPass.m_StencilClear = info.m_StencilClear;

		if (!renderPass.m_RenderTexture)
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

		std::stringstream str;
		str << "OpenGLDevice: Shader " << handle << " created.";
		Debug().LogInfo(str.str());

		return handle;
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
		pipeline.m_TextureCounter = 0;

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
		str << "OpenGLDevice: Compute pipeline " << handle << " created.";
		Debug().LogInfo(str.str());

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
				setLayout.m_BindingIndices.emplace_back(setLayoutInfo.m_Buffers[i].m_BindingIndex);
			setLayout.m_SamplerNames = std::move(setLayoutInfo.m_SamplerNames);
		}
		return iter->second;
	}

	DescriptorSetHandle OpenGLDevice::CreateDescriptorSet(DescriptorSetInfo&& setInfo)
	{
		std::vector<BufferHandle> bufferHandles;
		std::vector<TextureHandle> textureHandles;
		bufferHandles.resize(setInfo.m_Buffers.size());
		textureHandles.resize(setInfo.m_Samplers.size());

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
		set.m_Buffers = std::move(bufferHandles);
		set.m_Textures = std::move(textureHandles);
		set.m_Layout = setInfo.m_Layout;

		std::stringstream str;
		str << "OpenGLDevice: Descriptor set " << handle << " created.";
		Debug().LogInfo(str.str());

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
			glSet->m_Textures[samplerInfo.m_DescriptorIndex - setWriteInfo.m_Buffers.size()] = samplerInfo.m_TextureHandle;
		}
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

		std::stringstream str;
		str << "OpenGLDevice: Set layout " << handle << " was freed from device memory.";
		Debug().LogInfo(str.str());
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

		std::stringstream str;
		str << "OpenGLDevice: Set " << handle << " was freed from device memory.";
		Debug().LogInfo(str.str());
	}

	void OpenGLDevice::CreateRenderTexture(GL_RenderTexture& renderTexture)
	{
		/* Create framebuffer */
		glGenFramebuffers(1, &renderTexture.m_GLFramebufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindFramebuffer(GL_FRAMEBUFFER, renderTexture.m_GLFramebufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());

		const size_t numAttachments = renderTexture.m_Info.Attachments.size() + (renderTexture.m_Info.HasDepth ? 1 : 0) + (renderTexture.m_Info.HasStencil ? 1 : 0);
		renderTexture.m_AttachmentNames.resize(numAttachments);
		renderTexture.m_Textures.resize(numAttachments);

		SamplerSettings sampler;
		sampler.MipmapMode = Filter::F_None;
		sampler.MinFilter = Filter::F_Nearest;
		sampler.MagFilter = Filter::F_Nearest;

		size_t textureCounter = 0;
		for (size_t i = 0; i < renderTexture.m_Info.Attachments.size(); ++i)
		{
			Attachment attachment = renderTexture.m_Info.Attachments[i];
			renderTexture.m_Textures[i] = CreateTexture({ renderTexture.m_Info.Width, renderTexture.m_Info.Height, attachment.Format, attachment.InternalFormat, attachment.ImageType, attachment.m_Type, 0, 0, attachment.ImageAspect, sampler });
			renderTexture.m_AttachmentNames[i] = attachment.Name;
			++textureCounter;
		}

		size_t depthIndex = 0, stencilIndex = 0;
		if (renderTexture.m_Info.HasDepth)
		{
			depthIndex = textureCounter;
			renderTexture.m_Textures[depthIndex] = CreateTexture({ renderTexture.m_Info.Width, renderTexture.m_Info.Height, PixelFormat::PF_Depth, PixelFormat::PF_Depth32, ImageType::IT_2D, DataType::DT_UInt, 0, 0, ImageAspect::IA_Depth, sampler });
			renderTexture.m_AttachmentNames[depthIndex] = "Depth";
			++textureCounter;
		}

		if (renderTexture.m_Info.HasStencil)
		{
			stencilIndex = textureCounter;
			renderTexture.m_Textures[stencilIndex] = CreateTexture({ renderTexture.m_Info.Width, renderTexture.m_Info.Height, PixelFormat::PF_Stencil, PixelFormat::PF_R8Uint, ImageType::IT_2D, DataType::DT_UInt, 0, 0, ImageAspect::IA_Stencil, sampler });
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

#pragma endregion
}
