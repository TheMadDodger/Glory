#include "MonoScriptTumbnail.h"
#include "MonoEditorExtension.h"
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

	ImageData* MonoScriptTumbnail::GetTumbnail(const ResourceMeta* pResourceType)
	{
		if (!m_pScriptTumbnail)
		{
			LoaderModule* pLoader = Game::GetGame().GetEngine()->GetLoaderModule<ImageData>();
			m_pScriptTumbnail = (ImageData*)pLoader->Load("./EditorAssets/Mono/Code.png");
		}

		return m_pScriptTumbnail;
	}

	void MonoScriptTumbnail::OnFileDoubleClick(UUID uuid)
	{
		AssetLocation location;
		if (!EditorAssetDatabase::GetAssetLocation(uuid, location)) return;

		MonoEditorExtension::OpenCSharpProject();

		ProjectSpace* pProject = ProjectSpace::GetOpenProject();
		std::filesystem::path path = pProject->RootPath();
		path = path.append("Assets").append(location.Path);
		std::string cmd = "\"C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/Common7/IDE/devenv\" /edit " + path.string();
		system(cmd.c_str());
	}
}
