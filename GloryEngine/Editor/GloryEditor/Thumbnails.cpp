#include "Thumbnails.h"
#include "ThumbnailsRenderer.h"
#include "ThumbnailSceneSetup.h"
#include "EditorApplication.h"
#include "EditorAssetDatabase.h"
#include "EditorAssets.h"
#include "Importer.h"

#include <ResourceMeta.h>
#include <MaterialData.h>
#include <ImageData.h>
#include <Resources.h>
#include <GraphicsDevice.h>

namespace Glory::Editor
{
	Thumbnails::Thumbnails(EditorApplication* pApp):
		m_pApp(pApp), m_ThumbnailsRenderer(new ThumbnailsRenderer(pApp->GetEngine()))
	{
	}

	Thumbnails::~Thumbnails()
	{
	}

	void Thumbnails::Initialize()
	{
		m_ThumbnailsRenderer->LoadResources();
		SetupInternalRenderableThumbnails();
	}

	void Thumbnails::Update()
	{
		m_ThumbnailsRenderer->CheckRenders();
		Resources& resources = m_pApp->GetEngine()->GetResources();

		std::vector<UUID> finishedRenders;
		for (auto renderingThumb : m_CurrentRenderingThumbnails)
		{
			const std::filesystem::path cachedThumbnailPath = GenerateCachedThumbnailPath(renderingThumb.first);
			/* Check if the render finished */
			ImageData* pImage = m_ThumbnailsRenderer->GetRenderResult(renderingThumb.first);
			if (!pImage) continue;

			/* Save to a file */
			Importer::Export(cachedThumbnailPath, pImage);
			finishedRenders.push_back(renderingThumb.first);

			/* We can't use this image directly, the format may be off,
			 * the exported image will have been converted, so when it loads that next frame we get a correct image. */
			delete pImage;

			/* In case this is a rerender, we do have to remove the thumbnail from GPU memory */
			auto iter = m_CachedThumbnailTextureHandles.find(renderingThumb.first);
			GraphicsDevice* pDevice = m_pApp->GetEngine()->ActiveGraphicsDevice();

			if (iter != m_CachedThumbnailTextureHandles.end())
				pDevice->FreeTexture(iter->second);
		}
		for (auto id : finishedRenders)
			m_CurrentRenderingThumbnails.erase(id);
	}

	void Thumbnails::SetupInternalRenderableThumbnails()
	{
		RegisterRenderableThumbnail<MaterialData>(SetupMaterialScene, CanRenderMaterial);
		RegisterRenderableThumbnail<MeshData>(SetupMeshScene, CanRenderMesh);
		RegisterRenderableThumbnail<ImageData>(SetupImageScene, NULL, CustomRenderImage);
		RegisterRenderableThumbnail<TextureData>(SetupImageScene, CanRenderTexture, CustomRenderTexture);
	}

	bool Thumbnails::DrawThumbnailButton(UUID uuid, float size)
	{
		const ThumbnailData thumb = GetThumbnail(uuid);
		EditorRenderImpl* pRenderImpl = m_pApp->GetEditorPlatform().GetRenderImpl();
		if (thumb.Texture)
			return ImGui::ImageButton(pRenderImpl->GetTextureID(thumb.Texture), ImVec2(size, size));
		else if (!thumb.Icon.empty())
			return ImGui::Button(thumb.Icon.data(), { size, size });
		return false;
	}

	void Thumbnails::DrawThumbnail(UUID uuid, float size)
	{
		const ThumbnailData thumb = GetThumbnail(uuid);
		EditorRenderImpl* pRenderImpl = m_pApp->GetEditorPlatform().GetRenderImpl();
		if (thumb.Texture)
			ImGui::Image(pRenderImpl->GetTextureID(thumb.Texture), ImVec2(size, size));
		else if(!thumb.Icon.empty())
			ImGui::TextUnformatted(thumb.Icon.data());
	}

	void Thumbnails::SetDirty(UUID uuid)
	{
		auto iter = m_ResourceIDToThumbnailID.find(uuid);
		if (iter == m_ResourceIDToThumbnailID.end() || iter->second == 0ull) return;

		m_CachedThumbnailTextures.erase(uuid);
		m_ResourceIDToThumbnailID[uuid] = 0ull;

		const std::filesystem::path path = GenerateCachedThumbnailPath(uuid);
		std::filesystem::remove(path);

		Resources& resources = m_pApp->GetEngine()->GetResources();
		resources.UnloadResource(uuid);
	}

	void Thumbnails::RegisterRenderableThumbnail(uint32_t hashCode, std::function<void(Entity, UUID)> sceneSetup,
		std::function<bool(UUID)> canRender, std::function<void(UUID, GraphicsDevice*, Renderer*, uint32_t, CommandBufferHandle)> customRender)
	{
		m_ThumbnailsRenderer->RegisterRenderableThumbnail(hashCode, sceneSetup, canRender, customRender);
	}

	std::filesystem::path Thumbnails::GenerateCachedThumbnailPath(const UUID uuid) const
	{
		std::filesystem::path compiledPath = ProjectSpace::GetOpenProject()->CachePath();
		compiledPath.append("Thumbnails").append(std::to_string(uuid)).replace_extension(".png");
		return compiledPath;
	}

	ThumbnailData Thumbnails::GetThumbnail(const UUID uuid)
	{
		const std::filesystem::path cachedThumbnailPath = GenerateCachedThumbnailPath(uuid);
		EditorRenderImpl* pRenderImpl = m_pApp->GetEditorPlatform().GetRenderImpl();
		if (m_CurrentRenderingThumbnails.contains(uuid) && m_CurrentRenderingThumbnails.at(uuid))
		{
			TextureHandle texture = EditorAssets::GetTexture("file");
			return { texture, "" };
		}

		/* Find thumbnail in cache */
		auto iter = m_CachedThumbnailTextures.find(uuid);
		if (iter != m_CachedThumbnailTextures.end())
		{
			/* Use the cached thumbnail */
			return { m_CachedThumbnailTextureHandles.at(uuid), ""};
		}

		if (std::filesystem::exists(cachedThumbnailPath))
		{
			/* Load existing cache */
			ImportedResource resource = Importer::Import(cachedThumbnailPath);
			if (!resource) return { nullptr, "" };
			Resources& resources = m_pApp->GetEngine()->GetResources();
			Resource* pImage = *resource;
			resources.AddResource(&pImage);
			InternalTexture& texture = m_CachedThumbnailTextures[uuid] = { static_cast<ImageData*>(pImage), false };
			TextureHandle textureHandle = m_pApp->GetEngine()->ActiveGraphicsDevice()->CreateTexture(&texture);
			m_CachedThumbnailTextureHandles[uuid] = textureHandle;
			m_ResourceIDToThumbnailID[uuid] = pImage->GetUUID();
			return { textureHandle, "" };
		}

		ResourceMeta meta;
		EditorAssetDatabase::GetAssetMetadata(uuid, meta);
		if (m_ThumbnailsRenderer->IsResourceRenderable(meta.Hash()))
		{
			/* Request the thumbnail to be rendered */
			m_CurrentRenderingThumbnails.emplace(uuid, 1);
			m_ThumbnailsRenderer->QueueRenderThumbnail(meta.Hash(), uuid);
		}

		TextureHandle texture = EditorAssets::GetTexture("file");
		return { texture, "" };
	}
}
