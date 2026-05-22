#include "Thumbnails.h"
#include "ThumbnailRenderer.h"
#include "ThumbnailSceneSetup.h"
#include "EditorApplication.h"
#include "EditorAssetDatabase.h"
#include "EditorAssets.h"

#include <ResourceMeta.h>

namespace Glory::Editor
{
	Thumbnails::Thumbnails(EditorApplication* pApp):
		m_pApp(pApp), m_ThumbnailRenderer(new ThumbnailRenderer(pApp->GetEngine()))
	{
	}

	Thumbnails::~Thumbnails()
	{
	}

	void Thumbnails::Initialize()
	{
		m_ThumbnailRenderer->LoadResources();
	}

	void Thumbnails::Update()
	{
		m_ThumbnailRenderer->CheckRenders();
	}

	void Thumbnails::SetupInternalRenderableThumbnails()
	{
		//RegisterRenderableThumbnail<MaterialData>(SetupMaterialScene, CanRenderMaterial);
		//RegisterRenderableThumbnail<MeshData>(SetupMeshScene, CanRenderMesh);
	}

	void Thumbnails::DrawThumbnail(UUID uuid, float size)
	{
		/* Find thumbnail in cache */
		auto iter = m_CachedThumbnail.find(uuid);
		if (iter != m_CachedThumbnail.end())
		{
			/* Use the cached thumbnail */
			return;
		}

		const std::filesystem::path cachedThumbnailPath = GenerateCachedThumbnailPath(uuid);
		if (std::filesystem::exists(cachedThumbnailPath))
		{
			/* Load existing cache */
			return;
		}

		ResourceMeta meta;
		EditorAssetDatabase::GetAssetMetadata(uuid, meta);

		//TextureData* pImage = nullptr;
		if (m_ThumbnailRenderer->IsResourceRenderable(meta.Hash()))
		{
			/* Request the thumbnail to be rendered */
			//pImage = m_ThumbnailRenderer->QueueThumbnailForRendering(meta.Hash(), uuid);
			return;
		}

		EditorRenderImpl* pRenderImpl = m_pApp->GetEditorPlatform().GetRenderImpl();
		TextureHandle texture = EditorAssets::GetTexture("file");
		ImGui::Image(texture ? pRenderImpl->GetTextureID(texture) : NULL, ImVec2(size, size));
	}

	void Thumbnails::RegisterRenderableThumbnail(uint32_t hashCode,
		std::function<void(Entity, UUID)> sceneSetup, std::function<bool(UUID)> canRender)
	{
		m_ThumbnailRenderer->RegisterRenderableThumbnail(hashCode, sceneSetup, canRender);
	}

	std::filesystem::path Thumbnails::GenerateCachedThumbnailPath(const UUID uuid) const
	{
		std::filesystem::path compiledPath = ProjectSpace::GetOpenProject()->CachePath();
		compiledPath.append("Thumbnails").append(std::to_string(uuid)).replace_extension(".png");
		return compiledPath;
	}
}
