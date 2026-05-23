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
		}
		for (auto id : finishedRenders)
			m_CurrentRenderingThumbnails.erase(id);
	}

	void Thumbnails::SetupInternalRenderableThumbnails()
	{
		RegisterRenderableThumbnail<MaterialData>(SetupMaterialScene, CanRenderMaterial);
		RegisterRenderableThumbnail<MeshData>(SetupMeshScene, CanRenderMesh);
		RegisterRenderableThumbnail<ImageData>(SetupImageScene, NULL, CustomRenderImage);
		RegisterRenderableThumbnail<TextureData>(SetupImageScene, NULL, CustomRenderTexture);
	}

	void Thumbnails::DrawThumbnail(UUID uuid, float size)
	{
		const std::filesystem::path cachedThumbnailPath = GenerateCachedThumbnailPath(uuid);
		EditorRenderImpl* pRenderImpl = m_pApp->GetEditorPlatform().GetRenderImpl();
		if (m_CurrentRenderingThumbnails.contains(uuid) && m_CurrentRenderingThumbnails.at(uuid))
		{
			TextureHandle texture = EditorAssets::GetTexture("file");
			ImGui::Image(texture ? pRenderImpl->GetTextureID(texture) : NULL, ImVec2(size, size));
			return;
		}

		/* Find thumbnail in cache */
		auto iter = m_CachedThumbnailTextureHandles.find(uuid);
		if (iter != m_CachedThumbnailTextureHandles.end())
		{
			/* Use the cached thumbnail */
			ImGui::Image(iter->second ? pRenderImpl->GetTextureID(iter->second) : NULL, ImVec2(size, size));
			return;
		}

		if (std::filesystem::exists(cachedThumbnailPath))
		{
			/* Load existing cache */
			ImportedResource resource = Importer::Import(cachedThumbnailPath);
			if (!resource) return;
			Resources& resources = m_pApp->GetEngine()->GetResources();
			Resource* pImage = *resource;
			resources.AddResource(&pImage);
			InternalTexture& texture = m_CachedThumbnailTextures.emplace_back(static_cast<ImageData*>(pImage), false);
			TextureHandle textureHandle = m_pApp->GetEngine()->ActiveGraphicsDevice()->CreateTexture(&texture);
			m_CachedThumbnailTextureHandles.emplace(uuid, textureHandle);
			ImGui::Image(textureHandle ? pRenderImpl->GetTextureID(textureHandle) : NULL, ImVec2(size, size));
			return;
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
		ImGui::Image(texture ? pRenderImpl->GetTextureID(texture) : NULL, ImVec2(size, size));
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
}
