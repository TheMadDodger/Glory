#include "ThumbnailRenderer.h"
#include "Importer.h"

#include <Engine.h>
#include <InternalTexture.h>
#include <GraphicsDevice.h>
#include <Components.h>
#include <AssetManager.h>
#include <EditorMaterialManager.h>
#include <EditorPipelineManager.h>
#include <GScene.h>
#include <TextureData.h>
#include <MeshData.h>
#include <MaterialData.h>
#include <PipelineData.h>
#include <Renderer.h>
#include <EditorApplication.h>
#include <EditorAssetDatabase.h>

namespace Glory::Editor
{
	static constexpr glm::uvec2 ThumbnailResolution{ 256, 256 };
	const size_t ThumbnailDataSize = ThumbnailResolution.x*ThumbnailResolution.y*4;

	ThumbnailRenderer::ThumbnailRenderer(Engine* pEngine) : SceneManager(pEngine), m_RenderingIDs{ 0ull }
	{
	}

	ThumbnailRenderer::~ThumbnailRenderer()
	{
		m_ThumbnailRenderSetupCallbacks.clear();
		m_CanRenderThumbnailCallbacks.clear();

		for (ImageData* pImage : m_RenderResults)
		{
			delete m_RenderResultTextures.at(pImage->GetUUID());
			delete pImage;
		}
		m_RenderResults.clear();
		m_RenderResultTextures.clear();
	}

	void ThumbnailRenderer::RegisterRenderableThumbnail(uint32_t hashCode,
		std::function<void(Entity, UUID)> sceneSetup, std::function<bool(UUID)> canRender)
	{
		m_ThumbnailRenderSetupCallbacks.emplace(hashCode, sceneSetup);
		m_CanRenderThumbnailCallbacks.emplace(hashCode, canRender);
	}

	bool ThumbnailRenderer::IsResourceRenderable(uint32_t hashCode)
	{
		auto itor = m_ThumbnailRenderSetupCallbacks.find(hashCode);
		return itor != m_ThumbnailRenderSetupCallbacks.end();
	}

	bool ThumbnailRenderer::CanRenderThumbnail(uint32_t hashCode, UUID id)
	{
		return m_CanRenderThumbnailCallbacks.at(hashCode)(id);
	}

	TextureData* ThumbnailRenderer::QueueThumbnailForRendering(uint32_t hashCode, UUID id)
	{
		if (!m_CanRenderThumbnailCallbacks.at(hashCode)(id)) return nullptr;

		auto alreadyRendererItor = m_RenderResultTextures.find(id);
		if (alreadyRendererItor != m_RenderResultTextures.end())
			return alreadyRendererItor->second;

		if (m_PreviouslyRequestedThumbnails.find(id) != m_PreviouslyRequestedThumbnails.end()) return nullptr;

		auto itor = m_ThumbnailRenderSetupCallbacks.find(hashCode);
		assert(itor != m_ThumbnailRenderSetupCallbacks.end());

		m_PreviouslyRequestedThumbnails.insert(id);
		m_QueuedThumbnails.push({ hashCode, id });
		return nullptr;
	}

	void ThumbnailRenderer::LoadResources()
	{
		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
		if (!pDevice) return;

		ImportedResource materialSphere = Importer::Import("./EditorAssets/Models/MaterialSphere.obj");
		ImportedResource* sphereMesh = materialSphere.Child("Sphere Material 0");

		AssetManager& assets = m_pEngine->GetAssetManager();
		MaterialSphereMesh = static_cast<MeshData*>(**sphereMesh);
		assets.AddLoadedResource(MaterialSphereMesh);

		Renderer* pRenderer = m_pEngine->ActiveRenderer();
		if (!pRenderer) return;
		pRenderer = pRenderer->CreateSecondaryRenderer(MaxThumbnailsInFlight);
		SetRenderer(pRenderer);
		pRenderer->OnWindowResize({ ThumbnailResolution.x, ThumbnailResolution.y });

		GScene* pScene = NewScene("ThumbnailScene");
		Entity cameraEntity = pScene->CreateEmptyObject("Camera");
		CameraComponent& camera = cameraEntity.AddComponent<CameraComponent>();
		cameraEntity.GetComponent<Transform>().Position = glm::vec3(0.0f, 0.0f, 3.5f);

		Entity lights = pScene->CreateEmptyObject("Lights");

		Entity lightEntity = pScene->CreateEmptyObject("Light1");
		pScene->SetParent(lightEntity.GetEntityID(), lights.GetEntityID());
		LightComponent& light = lightEntity.AddComponent<LightComponent>();
		light.m_Type = LightType::Point;
		light.m_Shadows.m_Enable = false;
		light.m_Intensity = 1.0f;
		lightEntity.GetComponent<Transform>().Position = glm::vec3(-5.0f, 2.5f, 5.0f);

		Entity lightEntity2 = pScene->CreateEmptyObject("Light2");
		pScene->SetParent(lightEntity2.GetEntityID(), lights.GetEntityID());
		LightComponent& light2 = lightEntity2.AddComponent<LightComponent>();
		light2.m_Type = LightType::Point;
		light2.m_Shadows.m_Enable = false;
		light2.m_Intensity = 1.0f;
		lightEntity2.GetComponent<Transform>().Position = glm::vec3(4.0f, 4.f, -5.0f);

		pScene->GetRegistry().InvokeAll<CameraComponent>(Utils::ECS::InvocationType::OnEnableDraw);

		m_PixelCopyBuffer = pDevice->CreateBuffer(ThumbnailResolution.x*ThumbnailResolution.y*4, BufferType::BT_TransferWrite, BufferFlags::BF_Read);
	}

	void ThumbnailRenderer::CheckRenders()
	{
		AssetManager& assets = m_pEngine->GetAssetManager();

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
			auto iter = m_RenderResultTextures.find(m_RenderingIDs[i]);
			if (iter != m_RenderResultTextures.end())
			{
				ImageData* pImage = iter->second->GetImageData(&assets);
				pImage->SetPixels(std::move(pixels), ThumbnailDataSize);
				iter->second->SetDirty(true);
				m_RenderingIDs[i] = 0ull;
				continue;
			}

			ImageData* pImage = new ImageData(ThumbnailResolution.x, ThumbnailResolution.y, PixelFormat::PF_RGBA, PixelFormat::PF_R8G8B8A8Srgb, 4, std::move(pixels), ThumbnailDataSize);
			pImage->SetResourceUUID(m_RenderingIDs[i]);
			m_RenderResults.emplace_back(pImage);
			InternalTexture* pTexture = new InternalTexture(pImage);
			pTexture->SetResourceUUID(m_RenderingIDs[i]);
			m_RenderResultTextures.emplace(m_RenderingIDs[i], pTexture);

			m_RenderingIDs[i] = 0ull;
		}
	}

	void ThumbnailRenderer::RerenderThumbnail(UUID uuid)
	{
		auto iter = m_RenderResultTextures.find(uuid);
		if (iter == m_RenderResultTextures.end()) return;
		m_PreviouslyRequestedThumbnails.insert(uuid);
		ResourceMeta meta;
		EditorAssetDatabase::GetAssetMetadata(uuid, meta);
		m_QueuedThumbnails.push({ meta.Hash(), uuid });
	}

	GScene* ThumbnailRenderer::NewScene(const std::string& name, bool additive)
	{
		/* New scene is always additive in the editor */
		GScene* pNewScene = new GScene();
		m_pOpenScenes.emplace_back(pNewScene);
		pNewScene->SetManager(this);
		return pNewScene;
	}

	void ThumbnailRenderer::OnLoadScene(UUID uuid)
	{
		/* This scene manager does not support loading scenes! */
		throw new std::exception("ThumbnailRenderer does not support loading scenes!");
	}

	void ThumbnailRenderer::OnUnloadScene(GScene* pScene)
	{
	}

	void ThumbnailRenderer::OnUnloadAllScenes()
	{
	}

	void ThumbnailRenderer::SetupRenders()
	{
		if (m_QueuedThumbnails.empty()) return;
		for (size_t i = 0; i < MaxThumbnailsInFlight; ++i)
		{
			if (m_QueuedThumbnails.empty()) return;
			if (m_RenderingIDs[i]) continue;

			auto& pair = m_QueuedThumbnails.front();

			/* Reset scene */
			GScene* pScene = GetActiveScene();
			if (pScene->ChildCount(0) > 2)
				pScene->DestroyEntity(pScene->Child(0, 2));

			Entity root = pScene->CreateEmptyObject("Root");

			/* Setup scene */
			m_ThumbnailRenderSetupCallbacks.at(pair.first)(root, pair.second);

			/* Move camera to focus object */
			/* ... */

			/* Update and draw */
			m_pRenderer->BeginFrame();
			UpdateScene(pScene);
			DrawScene(pScene);
			
			/* Render */
			m_pRenderer->Draw();
			m_pRenderer->EndFrame();

			/* Set scene in use and pop the queue */
			m_RenderingIDs[i] = pair.second;
			m_QueuedThumbnails.pop();
		}
	}
}
