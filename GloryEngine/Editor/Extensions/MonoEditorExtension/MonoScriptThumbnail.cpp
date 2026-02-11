#include "MonoScriptThumbnail.h"
#include "MonoEditorExtension.h"
#include "EditorAssets.h"

#include <EditorApplication.h>
#include <EditorTextureData.h>
#include <Importer.h>
#include <Engine.h>
#include <EditorAssetDatabase.h>

namespace Glory::Editor
{
	MonoScriptThumbnail::MonoScriptThumbnail() : m_pScriptThumbnail(nullptr)
	{
	}

	MonoScriptThumbnail::~MonoScriptThumbnail()
	{
		if (!m_pScriptThumbnail) return;
		delete m_pScriptThumbnail;
		m_pScriptThumbnail = nullptr;
	}

	TextureData* MonoScriptThumbnail::GetThumbnail(const ResourceMeta* pResourceType)
	{
		if (!m_pScriptThumbnail)
		{
			ImportedResource resource = Importer::Import("./EditorAssets/Mono/Code.png");
			ImageData* pImageData = (ImageData*)*resource;
			resource.Cleanup();
			m_pScriptThumbnail = new EditorTextureData(pImageData);
		}

		return m_pScriptThumbnail;
	}

	void MonoScriptThumbnail::OnFileDoubleClick(UUID uuid)
	{
		AssetLocation location;
		if (!EditorAssetDatabase::GetAssetLocation(uuid, location)) return;

		/* Open project first before opening a single file */
		MonoEditorExtension::OpenCSharpProject();

		ProjectSpace* pProject = ProjectSpace::GetOpenProject();
		std::filesystem::path path = pProject->RootPath();
		path = path.append("Assets").append(location.Path);
		MonoEditorExtension::OpenFile(path);
	}
}
