#include "GloryRuntime.h"
#include "RuntimeMaterialManager.h"
#include "RuntimeShaderManager.h"

#include <Engine.h>
#include <Debug.h>
#include <AssetArchive.h>
#include <BinaryStream.h>
#include <GScene.h>
#include <SceneManager.h>
#include <AssetManager.h>
#include <AssetDatabase.h>
#include <ShaderSourceData.h>

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

	void GloryRuntime::LoadAssetDatabase(const std::filesystem::path& path)
	{
		if (!std::filesystem::exists(path))
		{
			m_pEngine->GetDebug().LogFatalError("Missing asset database");
			return;
		}

		auto itor = std::find(m_AppendedAssetDatabases.begin(), m_AppendedAssetDatabases.end(), path);
		if (itor != m_AppendedAssetDatabases.end()) return;

		m_AppendedAssetDatabases.push_back(path);
		BinaryFileStream file{ path, true };
		BinaryStream* pStream = &file;

		AssetDatabase& db = m_pEngine->GetAssetDatabase();
		while (!pStream->Eof())
		{
			ResourceMeta meta;
			AssetLocation location;
			pStream->Read(meta.Name());
			pStream->Read(meta.ID());
			pStream->Read(meta.Hash());
			pStream->Read(location.Path);
			pStream->Read(location.Index);
			db.SetAsset(location, meta);
		}
	}

	void GloryRuntime::LoadAssetGroup(const std::filesystem::path& path)
	{
		if (!std::filesystem::exists(path)) return;

		BinaryFileStream file{ path, true };
		AssetArchive archive{ &file };
		archive.Deserialize(m_pEngine);

		for (size_t i = 0; i < archive.Size(); ++i)
		{
			Resource* pResource = archive.Get(m_pEngine, i);
			m_pEngine->GetAssetManager().AddLoadedResource(pResource);
		}
	}

	void GloryRuntime::LoadShaderPack(const std::filesystem::path& path)
	{
		if (!std::filesystem::exists(path)) return;

		BinaryFileStream file{ path, true };
		AssetArchive archive{ &file };
		archive.Deserialize(m_pEngine);

		for (size_t i = 0; i < archive.Size(); ++i)
		{
			Resource* pResource = archive.Get(m_pEngine, i);
			ShaderSourceData* pShader = static_cast<ShaderSourceData*>(pResource);

			m_pEngine->GetShaderManager().AddShader(pShader);
		}
	}

	void GloryRuntime::LoadScene(const std::filesystem::path& path)
	{
		if (!std::filesystem::exists(path)) return;
		
		/* Load asset database */
		const std::filesystem::path rootPath = path.parent_path();
		std::filesystem::path dbPath = rootPath;
		dbPath.append("Assets.gcdb");
		LoadAssetDatabase(dbPath);

		/* Load shader pack */
		std::filesystem::path shaderPackPath = path;
		shaderPackPath.replace_extension("gcsp");
		LoadShaderPack(shaderPackPath);

		/* Load asset group */
		std::filesystem::path assetGroupPath = path;
		assetGroupPath.replace_extension("gcag");
		LoadAssetGroup(assetGroupPath);

		/* Load scene */
		std::filesystem::path scenePath = path;
		scenePath.replace_extension("gcs");
		LoadSceneOnly(scenePath);
	}

	void GloryRuntime::LoadSceneOnly(const std::filesystem::path& path)
	{
		if (!std::filesystem::exists(path))
		{
			m_pEngine->GetDebug().LogFatalError("Missing scene file");
			return;
		}

		BinaryFileStream file{ path, true };
		AssetArchive archive{ &file };
		archive.Deserialize(m_pEngine);

		if (archive.Size() == 0) return;

		Resource* pRoot = archive.Get(m_pEngine, 0);
		GScene* pScene = dynamic_cast<GScene*>(pRoot);
		if (!pScene) return;

		pScene->SetManager(m_pEngine->GetSceneManager());

		/* Have to make sure every components add and validate callbacks are called */
		pScene->GetRegistry().InvokeAll(Utils::ECS::InvocationType::OnAdd);
		pScene->GetRegistry().InvokeAll(Utils::ECS::InvocationType::OnValidate);

		m_pEngine->GetSceneManager()->AddOpenScene(pScene, pScene->GetUUID());
	}
}
