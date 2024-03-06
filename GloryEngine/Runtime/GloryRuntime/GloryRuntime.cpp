#include "GloryRuntime.h"
#include "RuntimeMaterialManager.h"
#include "RuntimeShaderManager.h"

#include <Engine.h>
#include <DisplayManager.h>
#include <RendererModule.h>
#include <GraphicsModule.h>
#include <WindowModule.h>

#include <GraphicsThread.h>
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
		m_MaterialManager(new RuntimeMaterialManager(pEngine)), m_ShaderManager(new RuntimeShaderManager(pEngine)),
		m_pRenderer(nullptr), m_pGraphics(nullptr)
	{
	}

	GloryRuntime::~GloryRuntime() = default;

	void GloryRuntime::Initialize()
	{
		m_pEngine->SetMaterialManager(m_MaterialManager.get());
		m_pEngine->SetShaderManager(m_ShaderManager.get());
		m_pEngine->Initialize();

		m_pEngine->GetGraphicsThread()->BindBeginAndEndRender(this);
		m_pRenderer = m_pEngine->GetMainModule<RendererModule>();
		m_pGraphics = m_pEngine->GetMainModule<GraphicsModule>();
		m_pWindows = m_pEngine->GetMainModule<WindowModule>();

		if (m_pWindows)
			m_pWindows->GetMainWindow()->SetSplashScreen("./Splash.bmp");

		if (m_DataPath.empty()) return;
		const std::filesystem::path dataPath = m_DataPath;
		if (!std::filesystem::exists(dataPath))
		{
			std::stringstream str;
			str << "Data path: " << dataPath << " not found";
			m_pEngine->GetDebug().LogError(str.str());
		}

		for (const auto& entry : std::filesystem::directory_iterator(dataPath))
		{
			const std::filesystem::path path = entry.path();
			if (path.extension().compare(".dat") != 0) continue;
			const std::string name = path.filename().replace_extension("").string();
			BinaryFileStream file{ path, true };
			BinaryStream* stream = &file;
			Version version;
			stream->Read(version);
			if (Version::Compare(version, CoreVersion) != 0)
			{
				std::stringstream str;
				std::string versionStr;
				version.GetVersionString(versionStr);
				str << name << " was compiled with a different Core/Runtime version: "
					<< versionStr << " current: " << GloryCoreVersion;
				m_pEngine->GetDebug().LogError(str.str());
				continue;
			}
			std::vector<char> data;
			stream->Read(data);
			m_pEngine->AddData(name, std::move(data));
		}

		m_pEngine->ProcessData();
	}

	void GloryRuntime::Run()
	{
		m_pEngine->StartThreads();
		m_pEngine->GetSceneManager()->Start();

		while (!m_pEngine->WantsToQuit())
		{
			m_pEngine->Update();
		}

		m_pEngine->Cleanup();
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

	Engine* GloryRuntime::GetEngine()
	{
		return m_pEngine;
	}

	void GloryRuntime::SetDataPath(const std::string& dataPath)
	{
		m_DataPath = dataPath;
	}

	void GloryRuntime::GraphicsThreadEndRender()
	{
		RenderTexture* pTexture = m_pEngine->GetDisplayManager().GetDisplayRenderTexture(0);
		m_pGraphics->Blit(pTexture);
		m_pGraphics->Swap();
	}
}
