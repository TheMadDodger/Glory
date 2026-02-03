#include "ThumbnailRenderer.h"
#include "Importer.h"

#include <Engine.h>
#include <InternalTexture.h>
#include <GraphicsDevice.h>
#include <Components.h>
#include <AssetManager.h>
#include <GScene.h>
#include <TextureData.h>
#include <MeshData.h>
#include <Renderer.h>

namespace Glory::Editor
{
	ThumbnailRenderer::ThumbnailRenderer(Engine* pEngine) : SceneManager(pEngine), m_RenderingIDs{ 0ull }
	{
	}

	ThumbnailRenderer::~ThumbnailRenderer()
	{
		m_ThumbnailRenderSetupCallbacks.clear();
	}

	void ThumbnailRenderer::RegisterRenderableThumbnail(uint32_t hashCode, std::function<void(Entity, UUID)> sceneSetup)
	{
		m_ThumbnailRenderSetupCallbacks.emplace(hashCode, sceneSetup);
	}

	bool ThumbnailRenderer::CanRenderThumbnail(uint32_t hashCode)
	{
		auto itor = m_ThumbnailRenderSetupCallbacks.find(hashCode);
		return itor != m_ThumbnailRenderSetupCallbacks.end();
	}

	TextureData* ThumbnailRenderer::QueueThumbnailForRendering(uint32_t hashCode, UUID id)
	{
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

		MaterialSphereMesh = static_cast<MeshData*>(**sphereMesh);
		m_pEngine->GetAssetManager().AddLoadedResource(MaterialSphereMesh);

		Renderer* pRenderer = m_pEngine->ActiveRenderer();
		if (!pRenderer) return;
		pRenderer = pRenderer->CreateSecondaryRenderer(MaxThumbnailsInFlight);
		SetRenderer(pRenderer);
		pRenderer->OnWindowResize({ 256, 256 });

		GScene* pScene = NewScene("ThumbnailScene");
		Entity cameraEntity = pScene->CreateEmptyObject("Camera");
		CameraComponent& camera = cameraEntity.AddComponent<CameraComponent>();
		cameraEntity.GetComponent<Transform>().Position = glm::vec3(0.0f, 0.0f, 3.5f);
		Entity lightEntity = pScene->CreateEmptyObject("Sun");
		LightComponent& light = lightEntity.AddComponent<LightComponent>();
		light.m_Type = LightType::Sun;
		light.m_Shadows.m_Enable = false;

		pScene->GetRegistry().InvokeAll<CameraComponent>(Utils::ECS::InvocationType::OnEnableDraw);

		m_PixelCopyBuffer = pDevice->CreateBuffer(256*256*4, BufferType::BT_TransferWrite, BufferFlags::BF_Read);
	}

	void ThumbnailRenderer::CheckRenders()
	{
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

			char* pixels = new char[256*256*4];
			pDevice->ReadBuffer(m_PixelCopyBuffer, pixels, 0, 256*256*4);
			ImageData* pImage = new ImageData(256, 256, PixelFormat::PF_RGBA, PixelFormat::PF_R8G8B8A8Srgb, 4, std::move(pixels), 256*256*4);
			pImage->SetResourceUUID(m_RenderingIDs[i]);
			m_RenderResults.emplace_back(pImage);
			InternalTexture* pTexture = new InternalTexture(pImage);
			pTexture->SetResourceUUID(m_RenderingIDs[i]);
			m_RenderResultTextures.emplace(m_RenderingIDs[i], pTexture);

			m_RenderingIDs[i] = 0ull;
		}
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
			UpdateScene(pScene);
			DrawScene(pScene);

			/* Render */
			m_pRenderer->Draw();

			/* Set scene in use and pop the queue */
			m_RenderingIDs[i] = pair.second;
			m_QueuedThumbnails.pop();
		}
	}
}
