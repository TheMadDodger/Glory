#include "ClusteredRendererModule.h"
#include <Engine.h>
#include <GraphicsModule.h>
#include <GPUResourceManager.h>
#include <FileLoaderModule.h>
#include <CameraManager.h>

namespace Glory
{
	ClusteredRendererModule::ClusteredRendererModule()
		: m_pClusterShaderFile(nullptr), m_pClusterShaderMaterial(nullptr), m_pClusterShaderMaterialData(nullptr), m_pScreenToViewSSBO(nullptr)
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
		createInfo.Attachments.push_back(Attachment("Normal", PixelFormat::PF_R8G8B8A8Srgb, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color));
		return pResourceManager->CreateRenderTexture(createInfo);
	}

	void ClusteredRendererModule::OnCameraResize(CameraRef camera)
	{
		// When the camera rendertexture resizes we need to generate a new grid of clusters for that camera
		GraphicsModule* pGraphics = m_pEngine->GetGraphicsModule();
		GPUResourceManager* pResourceManager = pGraphics->GetResourceManager();

		glm::uvec2 resolution = camera.GetResolution();
		glm::uvec3 gridSize = glm::vec3(m_GridSizeX, m_GridSizeY, NUM_DEPTH_SLICES);

		Buffer* pClusterSSBO = nullptr;
		if (!camera.GetUserData<Buffer>("ClusterSSBO", pClusterSSBO)) return; // Should not happen but just in case
		GenerateClusterSSBO(pClusterSSBO, camera, gridSize, resolution);
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
		m_pClusterCullLightShaderFile = (FileData*)m_pEngine->GetLoaderModule<FileData>()->Load("./Shaders/Compute/ClusterCullLight.shader", importSettings);
		m_pClusterCullLightMaterialData = new MaterialData({ m_pClusterCullLightShaderFile }, { ShaderType::ST_Compute });

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
		
		delete m_pClusterCullLightShaderFile;
		m_pClusterCullLightShaderFile = nullptr;

		delete m_pClusterShaderMaterialData;
		m_pClusterShaderMaterialData = nullptr;

		delete m_pMarkActiveClustersMaterialData;
		m_pMarkActiveClustersMaterialData = nullptr;

		delete m_pCompactClustersMaterialData;
		m_pCompactClustersMaterialData = nullptr;
		
		delete m_pClusterCullLightMaterialData;
		m_pClusterCullLightMaterialData = nullptr;

		delete m_pScreenMaterial;
		m_pScreenMaterial = nullptr;
	}

	void ClusteredRendererModule::OnThreadedInitialize()
	{
		GraphicsModule* pGraphics = m_pEngine->GetGraphicsModule();
		GPUResourceManager* pResourceManager = pGraphics->GetResourceManager();

		m_pScreenToViewSSBO = pResourceManager->CreateBuffer(sizeof(ScreenToView), GL_SHADER_STORAGE_BUFFER, GL_STATIC_COPY, 2);
		m_pScreenToViewSSBO->Assign(NULL);

		m_pLightsSSBO = pResourceManager->CreateBuffer(sizeof(PointLight) * MAX_LIGHTS, GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, 3);
		m_pLightsSSBO->Assign(NULL);

		m_pClusterShaderMaterial = pResourceManager->CreateMaterial(m_pClusterShaderMaterialData);
		m_pMarkActiveClustersMaterial = pResourceManager->CreateMaterial(m_pMarkActiveClustersMaterialData);
		m_pCompactClustersMaterial = pResourceManager->CreateMaterial(m_pCompactClustersMaterialData);
		m_pClusterCullLightMaterial = pResourceManager->CreateMaterial(m_pClusterCullLightMaterialData);
	}

	void ClusteredRendererModule::OnThreadedCleanup()
	{

	}

	void ClusteredRendererModule::OnRender(CameraRef camera, const RenderData& renderData, const std::vector<PointLight>& lights)
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

	void ClusteredRendererModule::OnDoScreenRender(CameraRef camera, const std::vector<PointLight>& lights, size_t width, size_t height, RenderTexture* pRenderTexture)
	{
		GraphicsModule* pGraphics = m_pEngine->GetGraphicsModule();

		CreateMesh();

		Buffer* pClusterSSBO = nullptr;
		Buffer* pLightIndexSSBO = nullptr;
		Buffer* pLightGridSSBO = nullptr;
		if (!camera.GetUserData("ClusterSSBO", pClusterSSBO)) return;
		if (!camera.GetUserData("LightIndexSSBO", pLightIndexSSBO)) return;
		if (!camera.GetUserData("LightGridSSBO", pLightGridSSBO)) return;

		glDisable(GL_DEPTH_TEST);

		//glBindFramebuffer(GL_FRAMEBUFFER, NULL);
		//OpenGLGraphicsModule::LogGLError(glGetError());
		glViewport(0, 0, width, height);
		OpenGLGraphicsModule::LogGLError(glGetError());

		size_t count = std::min(lights.size(), MAX_LIGHTS);
		m_pLightsSSBO->Assign(lights.data(), 0, count * sizeof(PointLight));

		glm::uvec2 resolution = camera.GetResolution();
		glm::uvec3 gridSize = glm::vec3(m_GridSizeX, m_GridSizeY, NUM_DEPTH_SLICES);
		float zNear = camera.GetNear();
		float zFar = camera.GetFar();

		size_t sizeX = (size_t)std::ceilf(resolution.x / (float)gridSize.x);
		ScreenToView screenToView;
		screenToView.ProjectionInverse = camera.GetProjectionInverse();
		screenToView.ViewInverse = camera.GetViewInverse();
		screenToView.ScreenDimensions = resolution;
		screenToView.TileSizes = glm::uvec4(gridSize.x, gridSize.y, gridSize.z, sizeX);
		screenToView.Scale = (float)gridSize.z / std::log2f(zFar / zNear);
		screenToView.Bias = -((float)gridSize.z * std::log2f(zNear) / std::log2f(zFar / zNear));

		m_pScreenToViewSSBO->Assign((void*)&screenToView);

		// Set material
		Material* pMaterial = pGraphics->UseMaterial(m_pScreenMaterial);

		pRenderTexture->BindAll(pMaterial);

		pMaterial->SetFloat("zNear", camera.GetNear());
		pMaterial->SetFloat("zFar", camera.GetFar());

		pClusterSSBO->Bind();
		m_pScreenToViewSSBO->Bind();
		m_pLightsSSBO->Bind();
		pLightIndexSSBO->Bind();
		pLightGridSSBO->Bind();

		// Draw the screen mesh
		glBindVertexArray(m_ScreenQuadVertexArrayID);
		OpenGLGraphicsModule::LogGLError(glGetError());

		// Draw the triangles !
		glDrawArrays(GL_TRIANGLES, 0, 6); // 2*3 indices starting at 0 -> 2 triangles
		OpenGLGraphicsModule::LogGLError(glGetError());

		pClusterSSBO->Unbind();
		m_pScreenToViewSSBO->Unbind();
		m_pLightsSSBO->Unbind();
		pLightIndexSSBO->Unbind();
		pLightGridSSBO->Unbind();

		glBindVertexArray(NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());

		// Reset render textures and materials
		//glBindFramebuffer(GL_FRAMEBUFFER, NULL);
		//glViewport(0, 0, width, height);
		glUseProgram(NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());

		glEnable(GL_DEPTH_TEST);
	}

	void ClusteredRendererModule::OnStartCameraRender(CameraRef camera, const std::vector<PointLight>& lights)
	{
		GraphicsModule* pGraphics = m_pEngine->GetGraphicsModule();
		GPUResourceManager* pResourceManager = pGraphics->GetResourceManager();

		Buffer* pClusterSSBO = nullptr;
		if (!camera.GetUserData<Buffer>("ClusterSSBO", pClusterSSBO))
		{
			glm::uvec2 resolution = camera.GetResolution();
			glm::uvec3 gridSize = glm::vec3(m_GridSizeX, m_GridSizeY, NUM_DEPTH_SLICES);

			Buffer* pActiveClustersSSBO = nullptr;
			Buffer* pActiveUniqueClustersSSBO = nullptr;
			Buffer* pLightIndexSSBO = nullptr;
			Buffer* pLightGridSSBO = nullptr;

			pClusterSSBO = pResourceManager->CreateBuffer(sizeof(VolumeTileAABB) * NUM_CLUSTERS, GL_SHADER_STORAGE_BUFFER, GL_STATIC_COPY, 1);
			pClusterSSBO->Assign(NULL);

			pActiveClustersSSBO = pResourceManager->CreateBuffer(sizeof(bool) * NUM_CLUSTERS, GL_SHADER_STORAGE_BUFFER, GL_STATIC_COPY, 1);
			pActiveClustersSSBO->Assign(NULL);

			pActiveUniqueClustersSSBO = pResourceManager->CreateBuffer(sizeof(uint32_t) * (NUM_CLUSTERS + 1), GL_SHADER_STORAGE_BUFFER, GL_STATIC_COPY, 2);
			pActiveUniqueClustersSSBO->Assign(NULL);

			pLightIndexSSBO = pResourceManager->CreateBuffer(sizeof(uint32_t) * (NUM_CLUSTERS * MAX_LIGHTS_PER_TILE + 1), GL_SHADER_STORAGE_BUFFER, GL_STATIC_COPY, 4);
			pLightIndexSSBO->Assign(NULL);
			
			pLightGridSSBO = pResourceManager->CreateBuffer(sizeof(LightGrid) * NUM_CLUSTERS, GL_SHADER_STORAGE_BUFFER, GL_STATIC_COPY, 5);
			pLightGridSSBO->Assign(NULL);

			camera.SetUserData("ClusterSSBO", pClusterSSBO);
			camera.SetUserData("ActiveClustersSSBO", pActiveClustersSSBO);
			camera.SetUserData("ActiveUniqueClustersSSBO", pActiveUniqueClustersSSBO);
			camera.SetUserData("LightIndexSSBO", pLightIndexSSBO);
			camera.SetUserData("LightGridSSBO", pLightGridSSBO);

			GenerateClusterSSBO(pClusterSSBO, camera, gridSize, resolution);
		}
	}

	void ClusteredRendererModule::OnEndCameraRender(CameraRef camera, const std::vector<PointLight>& lights)
	{
		GraphicsModule* pGraphics = m_pEngine->GetGraphicsModule();
		GPUResourceManager* pResourceManager = pGraphics->GetResourceManager();

		RenderTexture* pRenderTexture = CameraManager::GetRenderTextureForCamera(camera, m_pEngine);
		Texture* pDepthTexture = pRenderTexture->GetTextureAttachment("Depth");

		glm::uvec2 resolution = camera.GetResolution();
		glm::uvec3 gridSize = glm::vec3(m_GridSizeX, m_GridSizeY, NUM_DEPTH_SLICES);

		Buffer* pClusterSSBO = nullptr;
		Buffer* pActiveClustersSSBO = nullptr;
		Buffer* pActiveUniqueClustersSSBO = nullptr;
		Buffer* pLightIndexSSBO = nullptr;
		Buffer* pLightGridSSBO = nullptr;
		if (!camera.GetUserData("ClusterSSBO", pClusterSSBO)) return;
		if (!camera.GetUserData("ActiveClustersSSBO", pActiveClustersSSBO)) return;
		if (!camera.GetUserData("ActiveUniqueClustersSSBO", pActiveUniqueClustersSSBO)) return;
		if (!camera.GetUserData("LightIndexSSBO", pLightIndexSSBO)) return;
		if (!camera.GetUserData("LightGridSSBO", pLightGridSSBO)) return;

		//m_pMarkActiveClustersMaterial->Use();
		//pActiveClustersSSBO->Bind();
		//m_pMarkActiveClustersMaterial->SetFloat("zNear", camera.GetNear());
		//m_pMarkActiveClustersMaterial->SetFloat("zFar", camera.GetFar());
		//m_pMarkActiveClustersMaterial->SetUInt("tileSizeInPx", resolution.x / gridSize.x);
		//m_pMarkActiveClustersMaterial->SetUVec3("numClusters", gridSize);
		//m_pMarkActiveClustersMaterial->SetTexture("Depth", pDepthTexture);
		//pGraphics->DispatchCompute(resolution.x, resolution.y, 1);
		//pActiveClustersSSBO->Unbind();

		//m_pCompactClustersMaterial->Use();
		//pActiveClustersSSBO->Bind();
		//pActiveUniqueClustersSSBO->Bind();
		//pGraphics->DispatchCompute(NUM_CLUSTERS, 1, 1);
		//pActiveClustersSSBO->Unbind();
		//pActiveUniqueClustersSSBO->Unbind();

		size_t count = std::min(lights.size(), MAX_LIGHTS);
		m_pLightsSSBO->Assign(lights.data(), 0, count * sizeof(PointLight));

		float zNear = camera.GetNear();
		float zFar = camera.GetFar();

		size_t sizeX = (size_t)std::ceilf(resolution.x / (float)gridSize.x);
		ScreenToView screenToView;
		screenToView.ProjectionInverse = camera.GetProjectionInverse();
		screenToView.ViewInverse = camera.GetViewInverse();
		screenToView.ScreenDimensions = resolution;
		screenToView.TileSizes = glm::uvec4(gridSize.x, gridSize.y, gridSize.z, sizeX);
		screenToView.Scale = (float)gridSize.z / std::log2f(zFar / zNear);
		screenToView.Bias = -((float)gridSize.z * std::log2f(zNear) / std::log2f(zFar / zNear));

		m_pScreenToViewSSBO->Assign((void*)&screenToView);

		m_pClusterCullLightMaterial->Use();
		m_pClusterCullLightMaterial->SetMatrix4("viewMatrix", camera.GetView());
		pClusterSSBO->Bind();
		m_pScreenToViewSSBO->Bind();
		m_pLightsSSBO->Bind();
		pLightIndexSSBO->Bind();
		pLightGridSSBO->Bind();
		pGraphics->DispatchCompute(1, 1, 6);
		pClusterSSBO->Unbind();
		m_pScreenToViewSSBO->Unbind();
		m_pLightsSSBO->Unbind();
		pLightIndexSSBO->Unbind();
		pLightGridSSBO->Unbind();
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

	size_t ClusteredRendererModule::GetGCD(size_t a, size_t b)
	{
		if (b == 0)
			return a;
		return GetGCD(b, a % b);
	}

	void ClusteredRendererModule::GenerateClusterSSBO(Buffer* pBuffer, CameraRef camera, const glm::uvec3& gridSize, const glm::uvec2& resolution)
	{
		float zNear = camera.GetNear();
		float zFar = camera.GetFar();

		size_t sizeX = (size_t)std::ceilf(resolution.x / (float)gridSize.x);
		ScreenToView screenToView;
		screenToView.ProjectionInverse = camera.GetProjectionInverse();
		screenToView.ViewInverse = camera.GetViewInverse();
		screenToView.ScreenDimensions = resolution;
		screenToView.TileSizes = glm::uvec4(gridSize.x, gridSize.y, gridSize.z, sizeX);
		screenToView.Scale = (float)gridSize.z / std::log2f(zFar / zNear);
		screenToView.Bias = -((float)gridSize.z * std::log2f(zNear) / std::log2f(zFar / zNear));

		m_pScreenToViewSSBO->Assign((void*)&screenToView);

		m_pClusterShaderMaterial->Use();
		pBuffer->Bind();
		m_pScreenToViewSSBO->Bind();
		m_pClusterShaderMaterial->SetFloat("zNear", zNear);
		m_pClusterShaderMaterial->SetFloat("zFar", zFar);
		m_pEngine->GetGraphicsModule()->DispatchCompute(gridSize.x, gridSize.y, gridSize.z);
		pBuffer->Unbind();
		m_pScreenToViewSSBO->Unbind();
	}
}
