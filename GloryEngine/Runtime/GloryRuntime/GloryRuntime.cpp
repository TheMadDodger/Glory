#include "GloryRuntime.h"
#include "RuntimeMaterialManager.h"
#include "RuntimeShaderManager.h"

#include <Engine.h>
#include <AssetArchive.h>
#include <BinaryStream.h>
#include <GScene.h>
#include <SceneManager.h>
#include <AssetManager.h>

#include <filesystem>

namespace Glory
{
	GloryRuntime::GloryRuntime(Engine* pEngine): m_pEngine(pEngine),
		m_MaterialManager(new RuntimeMaterialManager(pEngine)), m_ShaderManager(new RuntimeShaderManager(pEngine))
	{
	}

	GloryRuntime::~GloryRuntime() = default;

	void GloryRuntime::Initialize()
	{
		m_pEngine->SetMaterialManager(m_MaterialManager.get());
		m_pEngine->SetShaderManager(m_ShaderManager.get());

		m_pEngine->Initialize();
	}

	void GloryRuntime::Run()
	{
		m_pEngine->StartThreads();
		m_pEngine->GetSceneManager()->Start();

		while (!m_pEngine->WantsToQuit())
		{
			m_pEngine->Update();
		}
	}

	void GloryRuntime::LoadScene(const std::filesystem::path& path)
	{
		if (!std::filesystem::exists(path))
			return;
		const std::filesystem::path rootPath = path.parent_path();
		std::filesystem::path dbPath = rootPath;
		dbPath.append("Assets.gcdb");

		BinaryFileStream file{ path, true };
		AssetArchive archive{ &file };
		archive.Deserialize(m_pEngine);

		if (archive.Size() == 0) return;

		Resource* pRoot = archive.Get(m_pEngine, 0);
		GScene* pScene = dynamic_cast<GScene*>(pRoot);
		if (!pScene) return;

		for (size_t i = 1; i < archive.Size(); ++i)
		{
			Resource* pResource = archive.Get(m_pEngine, i);
			m_pEngine->GetAssetManager().AddLoadedResource(pResource);
		}

		pScene->SetManager(m_pEngine->GetSceneManager());

		/* Have to make sure every components add and validate callbacks are called */
		pScene->GetRegistry().InvokeAll(Utils::ECS::InvocationType::OnAdd);
		pScene->GetRegistry().InvokeAll(Utils::ECS::InvocationType::OnValidate);

		m_pEngine->GetSceneManager()->AddOpenScene(pScene, pScene->GetUUID());
	}
}
