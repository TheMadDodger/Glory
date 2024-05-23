#include "MonoScriptTumbnail.h"
#include "MonoEditorExtension.h"
#include "EditorAssets.h"

#include <EditorApplication.h>
#include <EditorTextureData.h>
#include <Importer.h>
#include <Engine.h>
#include <EditorAssetDatabase.h>

namespace Glory::Editor
{
	MonoScriptTumbnail::MonoScriptTumbnail() : m_pScriptTumbnail(nullptr)
	{
	}

	MonoScriptTumbnail::~MonoScriptTumbnail()
	{
		if (!m_pScriptTumbnail) return;
		delete m_pScriptTumbnail;
		m_pScriptTumbnail = nullptr;
	}

	TextureData* MonoScriptTumbnail::GetTumbnail(const ResourceMeta* pResourceType)
	{
		if (!m_pScriptTumbnail)
		{
			ImportedResource resource = Importer::Import("./EditorAssets/Mono/Code.png");
			ImageData* pImageData = (ImageData*)*resource;
			resource.Cleanup();
			m_pScriptTumbnail = new EditorTextureData(pImageData);
		}

		return m_pScriptTumbnail;
	}

	void MonoScriptTumbnail::OnFileDoubleClick(UUID uuid)
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
