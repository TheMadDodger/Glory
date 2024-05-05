#include "GloryRuntime.h"
#include "RuntimeSceneManager.h"
#include "RuntimeMaterialManager.h"
#include "RuntimeShaderManager.h"
#include "RuntimePipelineManager.h"

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
		m_SceneManager(new RuntimeSceneManager(this)),
		m_PipelineManager(new RuntimePipelineManager(pEngine)),
		m_MaterialManager(new RuntimeMaterialManager(pEngine)),
		m_ShaderManager(new RuntimeShaderManager(pEngine)),
		m_pRenderer(nullptr), m_pGraphics(nullptr), m_pWindows(nullptr)
	{
	}

	GloryRuntime::~GloryRuntime() = default;

	void GloryRuntime::Initialize()
	{
		m_pEngine->SetSceneManager(m_SceneManager.get());
		m_pEngine->SetMaterialManager(m_MaterialManager.get());
		m_pEngine->SetShaderManager(m_ShaderManager.get());
		m_pEngine->SetPipelineManager(m_PipelineManager.get());
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
			return;
		}

		/* Load asset database */
		std::filesystem::path dbPath = dataPath;
		dbPath.append("Assets.gcdb");
		LoadAssetDatabase(dbPath);

		/* Load the shared asset pack */
		std::filesystem::path sharedPath = dataPath;
		sharedPath.append("Shared.gcag");
		if (std::filesystem::exists(sharedPath))
		{
			LoadAssetGroup(sharedPath);
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

		const UUID entryScene = m_pEngine->GetAssetDatabase().GetEntryScene();
		if (entryScene)
			LoadScene(entryScene);
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
		UUID entryScene;
		pStream->Read(entryScene);
		db.SetEntryScene(entryScene);

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

	void GloryRuntime::LoadScene(const UUID uuid)
	{
		m_pEngine->GetSceneManager()->OpenScene(uuid, false);
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

	Engine* GloryRuntime::GetEngine()
	{
		return m_pEngine;
	}

	void GloryRuntime::SetDataPath(const std::string& dataPath)
	{
		m_DataPath = dataPath;
	}

	std::string_view GloryRuntime::GetDataPath()
	{
		return m_DataPath;
	}

	void GloryRuntime::GraphicsThreadEndRender()
	{
		RenderTexture* pTexture = m_pEngine->GetDisplayManager().GetDisplayRenderTexture(0);
		m_pGraphics->Blit(pTexture);
		m_pGraphics->Swap();
	}
}
