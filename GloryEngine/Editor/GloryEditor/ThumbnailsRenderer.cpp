#include "ThumbnailsRenderer.h"
#include "Importer.h"
#include "EditorMaterialManager.h"
#include "EditorPipelineManager.h"
#include "EditorApplication.h"
#include "EditorAssetDatabase.h"
#include "ShaderSourceData.h"

#include <IEngine.h>
#include <InternalTexture.h>
#include <GraphicsDevice.h>
#include <Components.h>
#include <Resources.h>
#include <GScene.h>
#include <TextureData.h>
#include <MeshData.h>
#include <MaterialData.h>
#include <PipelineData.h>
#include <Renderer.h>
#include <CameraComponentManager.h>
#include <InternalPipeline.h>
#include <DescriptorHelpers.h>

namespace Glory::Editor
{
	static InternalPipeline ImagePipelineData;

	ThumbnailsRenderer::ThumbnailsRenderer(IEngine* pEngine) : SceneManager(pEngine), m_RenderingIDs{ 0ull }
	{
	}

	ThumbnailsRenderer::~ThumbnailsRenderer()
	{
		m_ThumbnailRenderSetupCallbacks.clear();
		m_CustomRenderThumbnailCallbacks.clear();

		for (auto pair : m_RenderResults)
			delete pair.second;
		m_RenderResults.clear();
	}

	void ThumbnailsRenderer::RegisterRenderableThumbnail(uint32_t hashCode, std::function<void(Entity, UUID)> sceneSetup,
		std::function<bool(UUID)> canRender, std::function<void(UUID, GraphicsDevice*, Renderer*, uint32_t, CommandBufferHandle)> customRender)
	{
		m_ThumbnailRenderSetupCallbacks.emplace(hashCode, sceneSetup);
		m_CanRenderThumbnailCallback.emplace(hashCode, canRender);
		m_CustomRenderThumbnailCallbacks.emplace(hashCode, customRender);
	}

	bool ThumbnailsRenderer::IsResourceRenderable(uint32_t hashCode)
	{
		auto itor = m_ThumbnailRenderSetupCallbacks.find(hashCode);
		return itor != m_ThumbnailRenderSetupCallbacks.end();
	}

	void ThumbnailsRenderer::QueueRenderThumbnail(uint32_t hashCode, UUID uuid)
	{
		m_References.emplace(uuid, uuid);
		m_ToQueueThumbnails.push_back({ hashCode, uuid });
	}

	ImageData* ThumbnailsRenderer::GetRenderResult(UUID uuid)
	{
		if (!m_RenderResults.contains(uuid)) return nullptr;
		ImageData* pImage = m_RenderResults.at(uuid);
		m_RenderResults.erase(uuid);
		return pImage;
	}

	void ThumbnailsRenderer::LoadResources()
	{
		RegisterComponentManagers();

		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
		if (!pDevice) return;

		ImportedResource materialSphere = Importer::Import("./EditorAssets/Models/MaterialSphere.obj");
		ImportedResource* sphereMesh = materialSphere.Child("Sphere Material 0");

		Resources& resources = m_pEngine->GetResources();
		MeshData* pSphereMesh = static_cast<MeshData*>(**sphereMesh);
		resources.AddResource(&pSphereMesh);
		MaterialSphereMeshID = pSphereMesh->GetUUID();

		ImportedResource vertexShader = Importer::Import("./EditorAssets/Shaders/Thumb_Vert.shader");
		ImportedResource fragmentShader = Importer::Import("./EditorAssets/Shaders/Thumb_Frag.shader");

		ShaderSourceData* pVertexSource = static_cast<ShaderSourceData*>(*vertexShader);
		ShaderSourceData* pFragmentSource = static_cast<ShaderSourceData*>(*fragmentShader);

		EditorPipelineManager& pipelines = static_cast<EditorPipelineManager&>(m_pEngine->GetPipelineManager());

		std::vector<FileData*> compiledShaders(2);
		std::vector<ShaderType> shaderTypes = { ShaderType::ST_Vertex, ShaderType::ST_Fragment };

		compiledShaders[0] = pipelines.CompileShader(pVertexSource);
		compiledShaders[1] = pipelines.CompileShader(pFragmentSource);

		ImagePipelineData.SetShaders(std::move(compiledShaders), std::move(shaderTypes));
		ImagePipelineData.SetDepthTestEnabled(false);
		ImagePipelineData.SetDepthWriteEnabled(false);
		ImagePipelineData.SetBlendEnabled(false);
		ImagePipelineData.GetCullFace() = CullFace::None;
		ImageSetLayoutHandle = CreateSamplerDescriptorLayout(pDevice, 1, { 0 }, { STF_Fragment }, { "Color" });
		ImageSetHandles.resize(MaxThumbnailsInFlight);
		for (size_t i = 0; i < MaxThumbnailsInFlight; ++i)
		{
			DescriptorSetInfo setInfo;
			setInfo.m_Layout = ImageSetLayoutHandle;
			setInfo.m_Samplers = { { NULL } };
			ImageSetHandles[i] = pDevice->CreateDescriptorSet(std::move(setInfo));
		}

		Renderer* pRenderer = m_pEngine->ActiveRenderer();
		if (!pRenderer) return;
		pRenderer = pRenderer->CreateSecondaryRenderer(MaxThumbnailsInFlight);
		SetRenderer(pRenderer);
		pRenderer->OnWindowResize({ ThumbnailResolution.x, ThumbnailResolution.y });

		ImagePipelineHandle = pDevice->CreatePipeline(pRenderer->FinalColorRenderPass(0), &ImagePipelineData, { ImageSetLayoutHandle },
			sizeof(glm::vec3), { AttributeType::Float3 });

		GScene* pScene = NewScene("ThumbnailScene");
		GetRegistryFactory().PopulateRegisry(pScene->GetRegistry());
		Entity cameraEntity = pScene->CreateEmptyObject("Camera");
		CameraComponent& camera = cameraEntity.AddComponent<CameraComponent>();
		cameraEntity.GetComponent<Transform>().Position = glm::vec3(0.0f, 0.0f, 100.0f);
		camera.m_Far = 500.0f;
		camera.m_Near = 0.01f;
		camera.m_HalfFOV = 60.0f;

		Entity lights = pScene->CreateEmptyObject("Lights");
		Entity sunEntity1 = pScene->CreateEmptyObject("Sun1");
		pScene->SetParent(sunEntity1.GetEntityID(), lights.GetEntityID());
		LightComponent& sun1 = sunEntity1.AddComponent<LightComponent>();
		sun1.m_Type = LightType::Sun;
		sun1.m_Shadows.m_Enable = false;
		sun1.m_Intensity = 1.0f;
		sunEntity1.GetComponent<Transform>().Rotation = glm::rotate(glm::identity<glm::quat>(), 45.0f, { -1.0f, 1.0f, 0.0f });

		Entity sunEntity2 = pScene->CreateEmptyObject("Sun2");
		pScene->SetParent(sunEntity2.GetEntityID(), lights.GetEntityID());
		LightComponent& sun2 = sunEntity2.AddComponent<LightComponent>();
		sun2.m_Type = LightType::Sun;
		sun2.m_Shadows.m_Enable = false;
		sun2.m_Intensity = 1.0f;
		sunEntity2.GetComponent<Transform>().Rotation = glm::rotate(glm::identity<glm::quat>(), 45.0f, { 1.0f, -1.0f, 0.0f });

		Utils::ECS::IComponentManager* manager = pScene->GetRegistry().GetComponentManager<CameraComponent>();
		manager->EnableDraw();

		m_PixelCopyBuffer = pDevice->CreateBuffer(ThumbnailResolution.x*ThumbnailResolution.y*4, BufferType::BT_TransferWrite, BufferFlags::BF_Read);
	}

	void ThumbnailsRenderer::CheckRenders()
	{
		Resources& assets = m_pEngine->GetResources();
		std::vector<size_t> toRemove;
		for (size_t i = 0; i < m_ToQueueThumbnails.size(); ++i)
		{
			auto& pair = m_ToQueueThumbnails[i];
			/* Check if loaded */
			Resource* pResource = assets.GetResource(pair.second);
			if (!pResource) continue;
			/* If there is a custom check to see if we can render run it here */
			auto iter = m_CanRenderThumbnailCallback.find(pair.first);
			if (iter != m_CanRenderThumbnailCallback.end() && iter->second && !iter->second(pair.second))
				continue;

			toRemove.push_back(i);
			m_QueuedThumbnails.push(std::move(m_ToQueueThumbnails[i]));
		}
		for (size_t i = 0; i < toRemove.size(); ++i)
			m_ToQueueThumbnails.erase(m_ToQueueThumbnails.begin() + toRemove[toRemove.size() - i - 1]);

		if (!GreyMaterialID)
		{
			EditorMaterialManager& materials = EditorApplication::GetInstance()->GetMaterialManager();
			EditorPipelineManager& pipelines = EditorApplication::GetInstance()->GetPipelineManager();

			const UUID phongPipeline = pipelines.FindPipeline(PipelineType::PT_Phong, false);
			MaterialData* pGreyMaterial = new MaterialData();
			assets.AddResource(&pGreyMaterial);
			GreyMaterialID = pGreyMaterial->GetUUID();
			PipelineData* pPipeline = pipelines.GetPipelineData(phongPipeline);
			pGreyMaterial->SetPipeline(phongPipeline);
			pPipeline->LoadIntoMaterial(pGreyMaterial);
			pGreyMaterial->Set<glm::vec4>("Color", glm::vec4(0.75f, 0.75f, 0.75f, 1.0f));
			pGreyMaterial->Set<float>("Shininess", 0.5f);
		}

		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
		if (!pDevice) return;

		SetupRenders();

		/* Check if any render finished */
		for (size_t i = 0; i < MaxThumbnailsInFlight; ++i)
		{
			if (!m_RenderingIDs[i]) continue;
			if (m_pRenderer->FrameBusy(i)) continue;

			TextureHandle finalColor = m_pRenderer->FinalColor(i);

			CommandBufferHandle commands = pDevice->Begin();
			pDevice->CopyImageToBuffer(commands, finalColor, m_PixelCopyBuffer);
			pDevice->End(commands);
			pDevice->Commit(commands);
			pDevice->Wait(commands);

			char* pixels = new char[ThumbnailDataSize];
			pDevice->ReadBuffer(m_PixelCopyBuffer, pixels, 0, ThumbnailDataSize);
			auto iter = m_RenderResults.find(m_RenderingIDs[i]);
			if (iter != m_RenderResults.end())
			{
				iter->second->SetPixels(std::move(pixels), ThumbnailDataSize);
				iter->second->SetDirty(false);
				m_RenderingIDs[i] = 0ull;
				continue;
			}

			m_RenderResults.emplace(m_RenderingIDs[i], new ImageData(ThumbnailResolution.x, ThumbnailResolution.y,
				PixelFormat::PF_R8G8B8A8Srgb, PixelFormat::PF_RGBA, 4, std::move(pixels), ThumbnailDataSize));
			m_References.erase(m_RenderingIDs[i]);

			m_RenderingIDs[i] = 0ull;
		}
	}

	GScene* ThumbnailsRenderer::NewScene(const std::string& name, bool additive)
	{
		/* New scene is always additive in the editor */
		GScene* pNewScene = new GScene();
		m_pOpenScenes.emplace_back(pNewScene);
		pNewScene->SetManager(this);
		return pNewScene;
	}

	void ThumbnailsRenderer::OnLoadScene(UUID uuid)
	{
		/* This scene manager does not support loading scenes! */
		throw new std::exception("ThumbnailsRenderer does not support loading scenes!");
	}

	void ThumbnailsRenderer::OnUnloadScene(GScene* pScene)
	{
	}

	void ThumbnailsRenderer::OnUnloadAllScenes()
	{
	}

	void ThumbnailsRenderer::SetupRenders()
	{
		if (m_QueuedThumbnails.empty()) return;
		for (size_t i = 0; i < MaxThumbnailsInFlight; ++i)
		{
			if (m_QueuedThumbnails.empty()) return;
			if (m_RenderingIDs[i]) continue;

			auto& pair = m_QueuedThumbnails.front();

			m_RenderingIDs[i] = pair.second;

			Renderer* pRenderer = GetRenderer();

			/* Use custom render if available */
			auto customRenderIter = m_CustomRenderThumbnailCallbacks.find(pair.first);
			if (customRenderIter != m_CustomRenderThumbnailCallbacks.end() && customRenderIter->second)
			{
				pRenderer->WaitForCurrentFrame();
				pRenderer->BeginFrame();
				CommandBufferHandle commandBuffer = pRenderer->BeginFrameCommands();
				const uint32_t frameIndex = pRenderer->GetCurrentFrameInFlight();
				GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
				customRenderIter->second(pair.second, pDevice, pRenderer, frameIndex, commandBuffer);
				std::vector<SemaphoreHandle> waitSemaphores, signalSemaphores;
				pRenderer->EndFrameCommands(waitSemaphores, signalSemaphores);
				pRenderer->EndFrame();
				m_QueuedThumbnails.pop();
				return;
			}

			/* Reset scene */
			GScene* pScene = GetActiveScene();
			if (pScene->ChildCount(0) > 2)
				pScene->DestroyEntity(pScene->Child(0, 2));

			Entity root = pScene->CreateEmptyObject("Root");

			/* Setup scene */
			m_ThumbnailRenderSetupCallbacks.at(pair.first)(root, pair.second);

			/* Move camera to focus object */
			Entity camera = Entity{ pScene->Child(0, 0), pScene };
			const BoundingSphere sphere = GenerateBoundingSphere(root, m_pEngine, root.GetComponent<Transform>().Position);

			/* Update and draw */
			//m_pEngine->GetDebug().StartCapture();
			pRenderer->BeginFrame();
			UpdateScene(pScene, 0.01f);
			camera.GetComponent<CameraComponent>().m_Camera.Focus(sphere);
 			DrawScene(pScene);

			/* Render */
			pRenderer->Draw();
			pRenderer->EndFrame();
			//m_pEngine->GetDebug().EndCapture();

			m_QueuedThumbnails.pop();

			pScene->DestroyEntity(root.GetEntityID());
		}
	}
}
