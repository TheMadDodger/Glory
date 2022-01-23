#include "ClusteredRendererModule.h"
#include <Engine.h>
#include <GraphicsModule.h>
#include <GPUResourceManager.h>
#include <FileLoaderModule.h>
#include <CameraManager.h>




#include <OGLRenderTexture.h>

namespace Glory
{
	ClusteredRendererModule::ClusteredRendererModule()
		: m_pClusterShaderFile(nullptr), m_pClusterShaderMaterial(nullptr), m_pClusterShaderMaterialData(nullptr),
		m_pClusterSSBO(nullptr), m_pScreenToViewSSBO(nullptr), m_ClusterGenerated(false)
	{
	}

	ClusteredRendererModule::~ClusteredRendererModule()
	{
	}

	RenderTexture* ClusteredRendererModule::CreateCameraRenderTexture(size_t width, size_t height)
	{
		GPUResourceManager* pResourceManager = m_pEngine->GetGraphicsModule()->GetResourceManager();
		RenderTextureCreateInfo createInfo(width, height, true);
		createInfo.Attachments.push_back(Attachment("Color", PixelFormat::PF_R8G8B8A8Srgb, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color));
		createInfo.Attachments.push_back(Attachment("Position", PixelFormat::PF_R8G8B8A8Srgb, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color));
		createInfo.Attachments.push_back(Attachment("Normal", PixelFormat::PF_R8G8B8A8Srgb, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color));
		return pResourceManager->CreateRenderTexture(createInfo);
	}

	void ClusteredRendererModule::PostInitialize()
	{
		FileImportSettings importSettings;
		importSettings.Flags = (int)(std::ios::ate | std::ios::binary);
		importSettings.AddNullTerminateAtEnd = true;
		m_pClusterShaderFile = (FileData*)m_pEngine->GetLoaderModule<FileData>()->Load("./Shaders/Compute/ClusterShader.shader", importSettings);
		m_pClusterShaderMaterialData = new MaterialData({ m_pClusterShaderFile }, { ShaderType::ST_Compute });
		m_pMarkActiveClustersShaderFile = (FileData*)m_pEngine->GetLoaderModule<FileData>()->Load("./Shaders/Compute/MarkActiveClusters.shader", importSettings);
		m_pMarkActiveClustersMaterialData = new MaterialData({ m_pMarkActiveClustersShaderFile }, { ShaderType::ST_Compute });
		m_pCompactClustersShaderFile = (FileData*)m_pEngine->GetLoaderModule<FileData>()->Load("./Shaders/Compute/BuildCompactClusterList.shader", importSettings);
		m_pCompactClustersMaterialData = new MaterialData({ m_pCompactClustersShaderFile }, { ShaderType::ST_Compute });

		FileData* pVert = (FileData*)m_pEngine->GetModule<FileLoaderModule>()->Load("./Shaders/ScreenRenderer.vert", importSettings);
		FileData* pFrag = (FileData*)m_pEngine->GetModule<FileLoaderModule>()->Load("./Shaders/ScreenRenderer.frag", importSettings);

		std::vector<FileData*> pShaderFiles = { pVert, pFrag };
		std::vector<ShaderType> shaderTypes = { ShaderType::ST_Vertex, ShaderType::ST_Fragment };

		m_pScreenMaterial = new MaterialData(pShaderFiles, shaderTypes);
	}

	void ClusteredRendererModule::Initialize()
	{
	}

	void ClusteredRendererModule::Cleanup()
	{
		delete m_pClusterShaderFile;
		m_pClusterShaderFile = nullptr;
		
		delete m_pMarkActiveClustersShaderFile;
		m_pMarkActiveClustersShaderFile = nullptr;
		
		delete m_pCompactClustersShaderFile;
		m_pCompactClustersShaderFile = nullptr;

		delete m_pClusterShaderMaterialData;
		m_pClusterShaderMaterialData = nullptr;

		delete m_pMarkActiveClustersMaterialData;
		m_pMarkActiveClustersMaterialData = nullptr;

		delete m_pCompactClustersMaterialData;
		m_pCompactClustersMaterialData = nullptr;
	}

	void ClusteredRendererModule::OnThreadedInitialize()
	{
		GraphicsModule* pGraphics = m_pEngine->GetGraphicsModule();
		GPUResourceManager* pResourceManager = pGraphics->GetResourceManager();

		m_pClusterSSBO = pResourceManager->CreateBuffer(sizeof(VolumeTileAABB) * NUMCLUSTERS, GL_SHADER_STORAGE_BUFFER, GL_STATIC_COPY, 1);
		m_pClusterSSBO->Assign(NULL);

		m_pScreenToViewSSBO = pResourceManager->CreateBuffer(sizeof(ScreenToView), GL_SHADER_STORAGE_BUFFER, GL_STATIC_COPY, 2);
		m_pScreenToViewSSBO->Assign(NULL);

		m_pActiveClustersSSBO = pResourceManager->CreateBuffer(sizeof(bool) * NUMCLUSTERS, GL_SHADER_STORAGE_BUFFER, GL_STATIC_COPY, 1);
		m_pActiveClustersSSBO->Assign(NULL);

		m_pActiveUniqueClustersSSBO = pResourceManager->CreateBuffer(sizeof(uint32_t) * (NUMCLUSTERS + 1), GL_SHADER_STORAGE_BUFFER, GL_STATIC_COPY, 2);
		m_pActiveUniqueClustersSSBO->Assign(NULL);

		m_pClusterShaderMaterial = pResourceManager->CreateMaterial(m_pClusterShaderMaterialData);
		m_pMarkActiveClustersMaterial = pResourceManager->CreateMaterial(m_pMarkActiveClustersMaterialData);
		m_pCompactClustersMaterial = pResourceManager->CreateMaterial(m_pCompactClustersMaterialData);
	}

	void ClusteredRendererModule::OnThreadedCleanup()
	{

	}

	void ClusteredRendererModule::OnRender(CameraRef camera, const RenderData& renderData)
	{
		GraphicsModule* pGraphics = m_pEngine->GetGraphicsModule();

		MeshData* pMeshData = nullptr;
		if (renderData.m_pModel == nullptr) return;
		pMeshData = renderData.m_pModel->GetMesh(renderData.m_MeshIndex);
		Material* pMaterial = pGraphics->UseMaterial(renderData.m_pMaterial);

		UniformBufferObjectTest ubo;
		ubo.model = renderData.m_World;
		ubo.view = camera.GetView();
		ubo.proj = camera.GetProjection();

		pMaterial->SetUBO(ubo);
		pMaterial->SetProperties();
		pMaterial->SetPropertiesExtra();
		pGraphics->DrawMesh(pMeshData);
	}

	void ClusteredRendererModule::OnDoScreenRender(size_t width, size_t height, RenderTexture* pRenderTexture)
	{
		GraphicsModule* pGraphics = m_pEngine->GetGraphicsModule();

		CreateMesh();

		glDisable(GL_DEPTH_TEST);

		//glBindFramebuffer(GL_FRAMEBUFFER, NULL);
		//OpenGLGraphicsModule::LogGLError(glGetError());
		glViewport(0, 0, width, height);
		OpenGLGraphicsModule::LogGLError(glGetError());

		// Set material
		OGLMaterial* pMaterial = (OGLMaterial*)pGraphics->UseMaterial(m_pScreenMaterial);
		GLTexture* pTexture = (GLTexture*)pRenderTexture->GetTextureAttachment(0);

		OGLRenderTexture* pGLRenderTexture = (OGLRenderTexture*)pRenderTexture;
		GLTexture* pGLTexture = (GLTexture*)pGLRenderTexture->GetTextureAttachment("Depth");

		pRenderTexture->BindAll(pMaterial);

		//pMaterial->SetTexture("Color", pGLTexture->GetID());

		// Draw the screen mesh
		glBindVertexArray(m_ScreenQuadVertexArrayID);
		OpenGLGraphicsModule::LogGLError(glGetError());

		// Draw the triangles !
		glDrawArrays(GL_TRIANGLES, 0, 6); // 2*3 indices starting at 0 -> 2 triangles
		OpenGLGraphicsModule::LogGLError(glGetError());

		pGraphics->Swap();

		glBindVertexArray(NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());

		// Reset render textures and materials
		//glBindFramebuffer(GL_FRAMEBUFFER, NULL);
		//glViewport(0, 0, width, height);
		glUseProgram(NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());

		glEnable(GL_DEPTH_TEST);
	}

	void ClusteredRendererModule::OnStartCameraRender(CameraRef camera)
	{
		GraphicsModule* pGraphics = m_pEngine->GetGraphicsModule();
		GPUResourceManager* pResourceManager = pGraphics->GetResourceManager();

		if (!m_ClusterGenerated)
		{
			glm::uvec2 resolution = camera.GetResolution();

			m_SizeX = (unsigned int)std::ceilf(resolution.x / (float)m_GridSizeX);
			ScreenToView screenToView;
			screenToView.inverseProjection = glm::inverse(camera.GetProjection());
			screenToView.screenDimensions = resolution;
			screenToView.tileSizes = glm::uvec4(m_GridSizeX, m_GridSizeY, m_GridSizeZ, m_SizeX);

			m_pScreenToViewSSBO->Assign((void*)&screenToView);

			m_pClusterShaderMaterial->Use();
			m_pClusterSSBO->Bind();
			m_pScreenToViewSSBO->Bind();
			m_pClusterShaderMaterial->SetFloat("zNear", camera.GetNear());
			m_pClusterShaderMaterial->SetFloat("zFar", camera.GetFar());
			pGraphics->DispatchCompute(m_GridSizeX, m_GridSizeY, m_GridSizeZ);
			m_pClusterSSBO->Unbind();
			m_pScreenToViewSSBO->Unbind();
			m_ClusterGenerated = true;
		}
	}

	void ClusteredRendererModule::OnEndCameraRender(CameraRef camera)
	{
		GraphicsModule* pGraphics = m_pEngine->GetGraphicsModule();
		GPUResourceManager* pResourceManager = pGraphics->GetResourceManager();

		glm::uvec2 resolution = camera.GetResolution();
		OGLRenderTexture* pRenderTexture = (OGLRenderTexture*)CameraManager::GetRenderTextureForCamera(camera, m_pEngine);
		GLTexture* pGLTexture = (GLTexture*)pRenderTexture->GetTextureAttachment("Depth");
		m_DepthBuffer = pGLTexture->GetID();

		glm::uvec3 numClusters = glm::uvec3(m_GridSizeX, m_GridSizeY, m_GridSizeZ);

		m_pMarkActiveClustersMaterial->Use();
		m_pActiveClustersSSBO->Bind();
		m_pMarkActiveClustersMaterial->SetFloat("zNear", camera.GetNear());
		m_pMarkActiveClustersMaterial->SetFloat("zFar", camera.GetFar());
		m_pMarkActiveClustersMaterial->SetUInt("tileSizeInPx", resolution.x / m_GridSizeX);
		m_pMarkActiveClustersMaterial->SetUVec3("numClusters", numClusters);
		((OGLMaterial*)m_pMarkActiveClustersMaterial)->SetTexture("depth", m_DepthBuffer);
		pGraphics->DispatchCompute(resolution.x, resolution.y, 1);
		m_pActiveClustersSSBO->Unbind();

		m_pCompactClustersMaterial->Use();
		m_pActiveClustersSSBO->Bind();
		m_pActiveUniqueClustersSSBO->Bind();
		pGraphics->DispatchCompute(NUMCLUSTERS, 1, 1);
		m_pActiveClustersSSBO->Unbind();
		m_pActiveUniqueClustersSSBO->Unbind();
	}


	void ClusteredRendererModule::CreateMesh()
	{
		if (m_HasMesh) return;

		static const GLfloat g_quad_vertex_buffer_data[] = {
		-1.0f, -1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f,
		};

		glGenVertexArrays(1, &m_ScreenQuadVertexArrayID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindVertexArray(m_ScreenQuadVertexArrayID);
		OpenGLGraphicsModule::LogGLError(glGetError());

		glGenBuffers(1, &m_ScreenQuadVertexbufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindBuffer(GL_ARRAY_BUFFER, m_ScreenQuadVertexbufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);
		OpenGLGraphicsModule::LogGLError(glGetError());

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		OpenGLGraphicsModule::LogGLError(glGetError());

		glBindBuffer(GL_ARRAY_BUFFER, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());

		glEnableVertexAttribArray(0);
		OpenGLGraphicsModule::LogGLError(glGetError());

		glBindVertexArray(NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());
		m_HasMesh = true;
	}

	void ClusteredRendererModule::CalculateActiveClusters()
	{
	}
}