#include "MonoScriptTumbnail.h"
#include "MonoEditorExtension.h"

#include <EditorApplication.h>
#include <Engine.h>
#include <EditorAssetDatabase.h>

namespace Glory::Editor
{
	MonoScriptTumbnail::MonoScriptTumbnail() : m_pScriptTumbnail(nullptr)
	{
	}

	MonoScriptTumbnail::~MonoScriptTumbnail()
	{
	}

	TextureData* MonoScriptTumbnail::GetTumbnail(const ResourceMeta* pResourceType)
	{
		if (!m_pScriptTumbnail)
		{
			LoaderModule* pLoader = EditorApplication::GetInstance()->GetEngine()->GetLoaderModule<ImageData>();
			m_pScriptTumbnail = (ImageData*)pLoader->Load("./EditorAssets/Mono/Code.png");
		}

		return (TextureData*)m_pScriptTumbnail->Subresource(0);
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
