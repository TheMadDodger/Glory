#include "GloryRuntime.h"
#include "RuntimeAssetManager.h"
#include "RuntimeSceneManager.h"
#include "RuntimeMaterialManager.h"
#include "RuntimePipelineManager.h"

#include <Engine.h>
#include <DisplayManager.h>
#include <RendererModule.h>
#include <GraphicsModule.h>
#include <WindowModule.h>

#include <Debug.h>
#include <AssetArchive.h>
#include <BinaryStream.h>
#include <GScene.h>
#include <SceneManager.h>
#include <AssetManager.h>
#include <AssetDatabase.h>

#include <filesystem>
#include <CRC.h>

namespace Glory
{
	GloryRuntime::GloryRuntime(Engine* pEngine): m_pEngine(pEngine),
		m_AssetManager(new RuntimeAssetManager(pEngine)),
		m_SceneManager(new RuntimeSceneManager(this)),
		m_PipelineManager(new RuntimePipelineManager(pEngine)),
		m_MaterialManager(new RuntimeMaterialManager(pEngine)),
		m_pRenderer(nullptr), m_pGraphics(nullptr), m_pWindows(nullptr),
		m_LastRenderedFrame(std::chrono::system_clock::now())
	{
	}

	GloryRuntime::~GloryRuntime() = default;

	void GloryRuntime::Initialize()
	{
		if (m_DataPath.empty())
		{
			m_pEngine->GetDebug().LogError("No data path provided");
		}
		const std::filesystem::path dataPath = m_DataPath;
		if (!std::filesystem::exists(dataPath))
		{
			std::stringstream str;
			str << "Data path: " << dataPath << " not found";
			m_pEngine->GetDebug().LogError(str.str());
			return;
		}

		/* Load data files */
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
			m_pEngine->AddData(path, name, std::move(data));
		}

		/* Initialize engine */
		m_pEngine->SetAssetManager(m_AssetManager.get());
		m_pEngine->SetSceneManager(m_SceneManager.get());
		m_pEngine->SetMaterialManager(m_MaterialManager.get());
		m_pEngine->SetPipelineManager(m_PipelineManager.get());
		m_pEngine->Initialize();

		m_pRenderer = m_pEngine->GetMainModule<RendererModule>();
		m_pGraphics = m_pEngine->GetMainModule<GraphicsModule>();
		m_pWindows = m_pEngine->GetMainModule<WindowModule>();

		/* Load splash screen */
		std::filesystem::path splashPath = m_DataPath;
		splashPath.append("Splash.bmp");

		if (!std::filesystem::exists(splashPath))
		{
			m_pEngine->GetDebug().LogFatalError("Missing splash screen!");
			return;
		}

		BinaryFileStream splashStream{ splashPath, true };
		std::vector<char> buffer{};
		buffer.resize(splashStream.Size());
		splashStream.Read(buffer.data(), buffer.size());

		static constexpr uint32_t checksum = 2749944603;
		const uint32_t crc = CRC::Calculate(buffer.data(), buffer.size(), CRC::CRC_32());

		if (checksum != crc)
		{
			m_pEngine->GetDebug().LogFatalError("Corrupt splash screen detected!");
			return;
		}

		if (m_pWindows)
			m_pWindows->GetMainWindow()->SetSplashScreen(buffer.data(), buffer.size());

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

		/* Load the shared shaders pack */
		sharedPath = dataPath;
		sharedPath.append("Shared.gcsp");
		if (std::filesystem::exists(sharedPath))
		{
			LoadShaderPack(sharedPath);
		}

		/* Process data files */
		m_pEngine->ProcessData();
	}

	void GloryRuntime::Run()
	{
		m_IsRunning = true;
		m_pEngine->GetSceneManager()->Start();

		while (!m_pEngine->WantsToQuit())
		{
			m_pEngine->Update();
			EndFrame();
		}

		m_IsRunning = false;
		m_pEngine->GetSceneManager()->Stop();
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
		m_pEngine->GetSceneManager()->LoadScene(uuid, false);
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
			FileData* pShader = static_cast<FileData*>(pResource);
			m_pEngine->GetPipelineManager().AddShader(pShader);
		}
	}

	Engine* GloryRuntime::GetEngine()
	{
		return m_pEngine;
	}

	void GloryRuntime::SetDataPath(const std::string& dataPath)
	{
		m_DataPath = dataPath;
		m_pEngine->SetRootPath(std::filesystem::path(dataPath).parent_path());
	}

	std::string_view GloryRuntime::GetDataPath()
	{
		return m_DataPath;
	}

	void GloryRuntime::SetFramerateLimit(float limit)
	{
		m_MaxFramerate = limit;
	}

	bool GloryRuntime::IsRunning() const
	{
		return m_IsRunning;
	}

	void GloryRuntime::EndFrame()
	{
		RenderTexture* pTexture = m_pEngine->GetDisplayManager().GetDisplayRenderTexture(0);
		m_pGraphics->Blit(pTexture);
		m_pGraphics->Swap();

		if (m_MaxFramerate == 0.0f)
		{
			m_LastRenderedFrame = std::chrono::system_clock::now();
			return;
		}

		/* Limit framerate */
		std::chrono::time_point<std::chrono::system_clock> currentTime = std::chrono::system_clock::now();
		const double frameIntervals = 1.0f / m_MaxFramerate;
		double timeSinceRefresh = 0.0f;

		while (timeSinceRefresh < frameIntervals)
		{
			currentTime = std::chrono::system_clock::now();
			std::chrono::duration<double> frameDuration = currentTime - m_LastRenderedFrame;
			timeSinceRefresh = std::chrono::duration_cast<std::chrono::nanoseconds>(frameDuration).count() / 1000000000.0;
		}
		m_LastRenderedFrame = std::chrono::system_clock::now();
	}
}
