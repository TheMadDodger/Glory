#include "ClusteredRendererModule.h"
#include <Engine.h>
#include <GraphicsModule.h>
#include <GPUResourceManager.h>
#include <FileLoaderModule.h>
#include <CameraManager.h>
#include <GloryContext.h>

namespace Glory
{
	GLORY_MODULE_VERSION_CPP(ClusteredRendererModule, 0, 1);

	ClusteredRendererModule::ClusteredRendererModule()
		: m_pClusterShaderData(nullptr), m_pClusterShaderMaterialData(nullptr), m_pClusterShaderMaterial(nullptr),
		m_pMarkActiveClustersShaderData(nullptr), m_pMarkActiveClustersMaterialData(nullptr), m_pMarkActiveClustersMaterial(nullptr),
		m_pCompactClustersShaderData(nullptr), m_pCompactClustersMaterialData(nullptr), m_pCompactClustersMaterial(nullptr),
		m_pClusterCullLightShaderData(nullptr), m_pClusterCullLightMaterialData(nullptr), m_pClusterCullLightMaterial(nullptr),
		m_pScreenToViewSSBO(nullptr), m_pLightsSSBO(nullptr), m_pScreenMaterial(nullptr)
	{
	}

	ClusteredRendererModule::~ClusteredRendererModule()
	{
	}

	void ClusteredRendererModule::GetCameraRenderTextureAttachments(std::vector<Attachment>& attachments)
	{
		attachments.push_back(Attachment("Color", PixelFormat::PF_RGBA, PixelFormat::PF_R8G8B8A8Srgb, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color));
		attachments.push_back(Attachment("Normal", PixelFormat::PF_RGBA, PixelFormat::PF_R8G8B8A8Srgb, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color));
		attachments.push_back(Attachment("Debug", PixelFormat::PF_RGBA, PixelFormat::PF_R8G8B8A8Srgb, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color));
	}

	void ClusteredRendererModule::OnCameraResize(CameraRef camera)
	{
		// When the camera rendertexture resizes we need to generate a new grid of clusters for that camera
		GraphicsModule* pGraphics = m_pEngine->GetGraphicsModule();
		GPUResourceManager* pResourceManager = pGraphics->GetResourceManager();

		Buffer* pClusterSSBO = nullptr;
		if (!camera.GetUserData<Buffer>("ClusterSSBO", pClusterSSBO)) return; // Should not happen but just in case
		GenerateClusterSSBO(pClusterSSBO, camera);
	}

	void ClusteredRendererModule::OnCameraPerspectiveChanged(CameraRef camera)
	{
		// When the camera changed perspective we need to generate a new grid of clusters for that camera
		GraphicsModule* pGraphics = m_pEngine->GetGraphicsModule();
		GPUResourceManager* pResourceManager = pGraphics->GetResourceManager();

		Buffer* pClusterSSBO = nullptr;
		if (!camera.GetUserData<Buffer>("ClusterSSBO", pClusterSSBO)) return; // Should not happen but just in case
		GenerateClusterSSBO(pClusterSSBO, camera);
	}

	void ClusteredRendererModule::PostInitialize()
	{
		FileImportSettings importSettings;
		importSettings.Flags = (int)(std::ios::ate | std::ios::binary);
		importSettings.AddNullTerminateAtEnd = true;

		// Cluster generator shader
		std::filesystem::path path;
		GetResourcePath("Shaders/Compute/ClusterShader.shader", path);
		FileData* pShaderFile = (FileData*)m_pEngine->GetLoaderModule<FileData>()->Load(path.string(), importSettings);
		m_pClusterShaderData = new ShaderSourceData(ShaderType::ST_Compute, pShaderFile);
		m_pClusterShaderMaterialData = new MaterialData({ m_pClusterShaderData });

		// Active cluster marker shader
		GetResourcePath("Shaders/Compute/MarkActiveClusters.shader", path);
		pShaderFile = (FileData*)m_pEngine->GetLoaderModule<FileData>()->Load(path.string(), importSettings);
		m_pMarkActiveClustersShaderData = new ShaderSourceData(ShaderType::ST_Compute, pShaderFile);
		m_pMarkActiveClustersMaterialData = new MaterialData({ m_pMarkActiveClustersShaderData });

		// Compact active clusters shader
		GetResourcePath("Shaders/Compute/BuildCompactClusterList.shader", path);
		pShaderFile = (FileData*)m_pEngine->GetLoaderModule<FileData>()->Load(path.string(), importSettings);
		m_pCompactClustersShaderData = new ShaderSourceData(ShaderType::ST_Compute, pShaderFile);
		m_pCompactClustersMaterialData = new MaterialData({ m_pCompactClustersShaderData });

		// Light culling shader
		GetResourcePath("Shaders/Compute/ClusterCullLight.shader", path);
		pShaderFile = (FileData*)m_pEngine->GetLoaderModule<FileData>()->Load(path.string(), importSettings);
		m_pClusterCullLightShaderData = new ShaderSourceData(ShaderType::ST_Compute, pShaderFile);
		m_pClusterCullLightMaterialData = new MaterialData({ m_pClusterCullLightShaderData });

		// Screen shaders
		GetResourcePath("Shaders/ScreenRenderer_Vert.shader", path);
		FileData* pVert = (FileData*)m_pEngine->GetModule<FileLoaderModule>()->Load(path.string(), importSettings);
		GetResourcePath("Shaders/ScreenRenderer_Frag.shader", path);
		FileData* pFrag = (FileData*)m_pEngine->GetModule<FileLoaderModule>()->Load(path.string(), importSettings);

		std::vector<ShaderSourceData*> pShaderFiles = { new ShaderSourceData(ShaderType::ST_Vertex, pVert), new ShaderSourceData(ShaderType::ST_Fragment, pFrag) };
		m_pScreenMaterial = new MaterialData(pShaderFiles);
	}

	void ClusteredRendererModule::Cleanup()
	{
		delete m_pClusterShaderData;
		m_pClusterShaderData = nullptr;

		delete m_pMarkActiveClustersShaderData;
		m_pMarkActiveClustersShaderData = nullptr;

		delete m_pCompactClustersShaderData;
		m_pCompactClustersShaderData = nullptr;

		delete m_pClusterCullLightShaderData;
		m_pClusterCullLightShaderData = nullptr;

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

		m_pScreenToViewSSBO = pResourceManager->CreateBuffer(sizeof(ScreenToView), BufferBindingTarget::B_SHADER_STORAGE, MemoryUsage::MU_STATIC_COPY, 2);
		m_pScreenToViewSSBO->Assign(NULL);

		m_pLightsSSBO = pResourceManager->CreateBuffer(sizeof(PointLight) * MAX_LIGHTS, BufferBindingTarget::B_SHADER_STORAGE, MemoryUsage::MU_STATIC_DRAW, 3);
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

		MeshData* pMeshData = renderData.m_pMesh;
		if (pMeshData == nullptr) return;
		Material* pMaterial = pGraphics->UseMaterial(renderData.m_pMaterial);
		if (!pMaterial) return;

		ObjectData object;
		object.Model = renderData.m_World;
		object.View = camera.GetView();
		object.Projection = camera.GetProjection();
		object.ObjectID = renderData.m_ObjectID;

		pMaterial->SetProperties();
		pMaterial->SetObjectData(object);
		pGraphics->DrawMesh(pMeshData, 0 , pMeshData->VertexCount());
	}

	void ClusteredRendererModule::OnDoScreenRender(CameraRef camera, const FrameData<PointLight>& lights, uint32_t width, uint32_t height, RenderTexture* pRenderTexture)
	{
		GraphicsModule* pGraphics = m_pEngine->GetGraphicsModule();

		Buffer* pClusterSSBO = nullptr;
		Buffer* pLightIndexSSBO = nullptr;
		Buffer* pLightGridSSBO = nullptr;
		if (!camera.GetUserData("ClusterSSBO", pClusterSSBO)) return;
		if (!camera.GetUserData("LightIndexSSBO", pLightIndexSSBO)) return;
		if (!camera.GetUserData("LightGridSSBO", pLightGridSSBO)) return;

		pGraphics->EnableDepthTest(false);
		pGraphics->SetViewport(0, 0, width, height);

		uint32_t count = (uint32_t)std::fmin(lights.size(), MAX_LIGHTS);
		m_pLightsSSBO->Assign(lights.data(), 0, count * sizeof(PointLight));

		glm::uvec2 resolution = camera.GetResolution();
		glm::uvec3 gridSize = glm::vec3(m_GridSizeX, m_GridSizeY, NUM_DEPTH_SLICES);
		float zNear = camera.GetNear();
		float zFar = camera.GetFar();

		const uint32_t sizeX = std::max((uint32_t)std::ceilf(resolution.x / (float)gridSize.x), (uint32_t)std::ceilf(resolution.y / (float)gridSize.y));
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

		// Draw the triangles !
		pGraphics->DrawScreenQuad();

		pClusterSSBO->Unbind();
		m_pScreenToViewSSBO->Unbind();
		m_pLightsSSBO->Unbind();
		pLightIndexSSBO->Unbind();
		pLightGridSSBO->Unbind();

		// Reset render textures and materials
		pGraphics->UseMaterial(nullptr);
		pGraphics->EnableDepthTest(true);
	}

	void ClusteredRendererModule::OnStartCameraRender(CameraRef camera, const FrameData<PointLight>& lights)
	{
		GraphicsModule* pGraphics = m_pEngine->GetGraphicsModule();
		GPUResourceManager* pResourceManager = pGraphics->GetResourceManager();

		Buffer* pClusterSSBO = nullptr;
		if (!camera.GetUserData<Buffer>("ClusterSSBO", pClusterSSBO))
		{
			Buffer* pActiveClustersSSBO = nullptr;
			Buffer* pActiveUniqueClustersSSBO = nullptr;
			Buffer* pLightIndexSSBO = nullptr;
			Buffer* pLightGridSSBO = nullptr;

			pClusterSSBO = pResourceManager->CreateBuffer(sizeof(VolumeTileAABB) * NUM_CLUSTERS, BufferBindingTarget::B_SHADER_STORAGE, MemoryUsage::MU_STATIC_COPY, 1);
			pClusterSSBO->Assign(NULL);

			pActiveClustersSSBO = pResourceManager->CreateBuffer(sizeof(bool) * NUM_CLUSTERS, BufferBindingTarget::B_SHADER_STORAGE, MemoryUsage::MU_STATIC_COPY, 1);
			pActiveClustersSSBO->Assign(NULL);

			pActiveUniqueClustersSSBO = pResourceManager->CreateBuffer(sizeof(uint32_t) * (NUM_CLUSTERS + 1), BufferBindingTarget::B_SHADER_STORAGE, MemoryUsage::MU_STATIC_COPY, 2);
			pActiveUniqueClustersSSBO->Assign(NULL);

			pLightIndexSSBO = pResourceManager->CreateBuffer(sizeof(uint32_t) * (NUM_CLUSTERS * MAX_LIGHTS_PER_TILE + 1), BufferBindingTarget::B_SHADER_STORAGE, MemoryUsage::MU_STATIC_COPY, 4);
			pLightIndexSSBO->Assign(NULL);

			pLightGridSSBO = pResourceManager->CreateBuffer(sizeof(LightGrid) * NUM_CLUSTERS, BufferBindingTarget::B_SHADER_STORAGE, MemoryUsage::MU_STATIC_COPY, 5);
			pLightGridSSBO->Assign(NULL);

			camera.SetUserData("ClusterSSBO", pClusterSSBO);
			camera.SetUserData("ActiveClustersSSBO", pActiveClustersSSBO);
			camera.SetUserData("ActiveUniqueClustersSSBO", pActiveUniqueClustersSSBO);
			camera.SetUserData("LightIndexSSBO", pLightIndexSSBO);
			camera.SetUserData("LightGridSSBO", pLightGridSSBO);

			GenerateClusterSSBO(pClusterSSBO, camera);
		}
	}

	void ClusteredRendererModule::OnEndCameraRender(CameraRef camera, const FrameData<PointLight>& lights)
	{
		GraphicsModule* pGraphics = m_pEngine->GetGraphicsModule();
		GPUResourceManager* pResourceManager = pGraphics->GetResourceManager();

		RenderTexture* pRenderTexture = GloryContext::GetCameraManager()->GetRenderTextureForCamera(camera, m_pEngine);
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

		uint32_t count = (uint32_t)std::fmin(lights.size(), MAX_LIGHTS);
		m_pLightsSSBO->Assign(lights.data(), 0, count * sizeof(PointLight));

		float zNear = camera.GetNear();
		float zFar = camera.GetFar();

		const uint32_t sizeX = std::max((uint32_t)std::ceilf(resolution.x / (float)gridSize.x), (uint32_t)std::ceilf(resolution.y / (float)gridSize.y));
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

	size_t ClusteredRendererModule::GetGCD(size_t a, size_t b)
	{
		if (b == 0)
			return a;
		return GetGCD(b, a % b);
	}

	void ClusteredRendererModule::GenerateClusterSSBO(Buffer* pBuffer, CameraRef camera)
	{
		const glm::uvec2 resolution = camera.GetResolution();
		const glm::uvec3 gridSize = glm::vec3(m_GridSizeX, m_GridSizeY, NUM_DEPTH_SLICES);

		const float zNear = camera.GetNear();
		const float zFar = camera.GetFar();

		const uint32_t sizeX = std::max((uint32_t)std::ceilf(resolution.x / (float)gridSize.x), (uint32_t)std::ceilf(resolution.y / (float)gridSize.y));
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
