#include "SteamAudioExtension.h"
#include "AudioSceneWindow.h"
#include "SoundMaterialEditor.h"
#include "SoundOccluderEditor.h"
#include "SoundMaterialImporter.h"
#include "AudioSceneTools.h"

#include <Debug.h>
#include <Engine.h>
#include <AudioComponents.h>
#include <SceneManager.h>
#include <SteamAudioModule.h>
#include <AudioScene.h>
#include <BinaryStream.h>
#include <MeshData.h>
#include <SoundComponents.h>
#include <AssetArchive.h>

#include <EntitySceneObjectEditor.h>
#include <CreateEntityObjectsCallbacks.h>
#include <CreateObjectAction.h>
#include <EntityEditor.h>
#include <MainEditor.h>
#include <EditorApplication.h>
#include <EditorSceneManager.h>
#include <EditorAssetDatabase.h>
#include <MenuBar.h>
#include <Shortcuts.h>
#include <Dispatcher.h>
#include <Package.h>
#include <EntitySceneObjectEditor.h>
#include <CreateEntityObjectsCallbacks.h>
#include <CreateObjectAction.h>

#include <IconsFontAwesome6.h>

EXTENSION_CPP(SteamAudioExtension)

namespace Glory::Editor
{
	SoundMaterialEditor MaterialEditor;
	SoundOccluderEditor OccluderCompEditor;
	SoundMaterialImporter Importer;

	static constexpr char* Shortcut_Window_SteamAudio = "Open Steam Audio Window";

	CREATE_OBJECT_CALLBACK_CPP(SoundOccluder, SoundOccluder, ());

	void SteamAudioExtension::HandleBeforeStart(Module* pModule)
	{
		/* Build audio occlusion scene and send to SteamAudio */
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		SteamAudioModule* pSteamAudio = pEngine->GetOptionalModule<SteamAudioModule>();
		if (!pSteamAudio) return;
		SceneManager* sceneManager = pEngine->GetSceneManager();

		ProjectSpace* pProject = ProjectSpace::GetOpenProject();
		std::filesystem::path cachedScenesPath = pProject->CachePath();
		cachedScenesPath.append("AudioScenes");
		if (!std::filesystem::exists(cachedScenesPath)) return;

		for (size_t i = 0; i < sceneManager->OpenScenesCount(); ++i)
		{
			GScene* pScene = sceneManager->GetOpenScene(i);
			std::filesystem::path cachedScenePath = cachedScenesPath;
			cachedScenePath.append(std::to_string(pScene->GetUUID())).replace_extension(".gcas");
			if (!std::filesystem::exists(cachedScenePath)) continue;
			BinaryFileStream fileStream{ cachedScenePath, true };
			AudioScene audioScene{ pScene->GetUUID() };
			audioScene.Deserialize(fileStream);
			pSteamAudio->AddAudioScene(std::move(audioScene));
		}

		pSteamAudio->RebuildAudioSimulationScene();
	}

	void SteamAudioExtension::HandleStop(Module* pModule)
	{
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		SteamAudioModule* pAudio = pEngine->GetOptionalModule<SteamAudioModule>();
		pAudio->RemoveAllAudioScenes();
	}

	SteamAudioExtension::SteamAudioExtension()
	{
	}

	SteamAudioExtension::~SteamAudioExtension()
	{
	}

	bool PackageAudioScenesTask(Engine* pEngine, const std::filesystem::path& packageRoot, PackageTaskState& task)
	{
		SteamAudioModule* pModule = pEngine->GetOptionalModule<SteamAudioModule>();

		/* Open every scene and package them individually along with their assets */
		for (size_t i = 0; i < NumScenesToPackage(); ++i)
		{
			const UUID sceneID = SceneToPackage(i);

			std::filesystem::path path = packageRoot;
			path.append("Data").append(std::to_string(sceneID)).replace_extension("gcs");
			{
				if (!std::filesystem::exists(path))
				{
					std::stringstream str;
					str << "Could not package audio scene: Scene " << sceneID << " packaged scene file not found";
					pEngine->GetDebug().LogError(str.str());
					task.m_SubTaskName = "";
					++task.m_ProcessedSubTasks;
					continue;
				}

				ResourceMeta meta;
				EditorAssetDatabase::GetAssetMetadata(sceneID, meta);
				task.m_SubTaskName = meta.Name();
				GScene* pScene = LoadedSceneToPackage(i);
				AudioScene audioScene{ pScene->GetUUID() };
				if (!GenerateAudioScene(pEngine, pScene, &pModule->DefaultMaterial(), audioScene))
					continue;

				/* Append the audio scene to the scenes archive */
				BinaryFileStream sceneFile{ path, false, false };
				sceneFile.Seek(0, BinaryStream::Relative::End);
				AssetArchive archive{ &sceneFile, AssetArchiveFlags::Write };
				AudioSceneData audioSceneData{ std::move(audioScene) };
				archive.Serialize(&audioSceneData);

				task.m_SubTaskName = "";
			}
			++task.m_ProcessedSubTasks;
		}

		return true;
	}

	void SteamAudioExtension::Initialize()
	{
		EditorApplication* pApp = EditorApplication::GetInstance();
		Engine* pEngine = pApp->GetEngine();
		Reflect::SetReflectInstance(&pEngine->Reflection());

		pEngine->GetSceneManager()->ComponentTypesInstance();

		EditorPlayer::RegisterLoopHandler(this);

		MenuBar::AddMenuItem("Window/Steam Audio", [pApp]() { pApp->GetMainEditor().GetWindow<AudioSceneWindow>(); }, NULL, Shortcut_Window_SteamAudio);

		Editor::RegisterEditor(&MaterialEditor);
		Editor::RegisterEditor(&OccluderCompEditor);
		Importer::Register(&Importer);

		EntitySceneObjectEditor::AddComponentIcon<SoundOccluder>(ICON_FA_EAR_DEAF);

		OBJECT_CREATE_MENU(SoundOccluder, SoundOccluder);

		GatherPackageTasksEvents().AddListener([&](const EmptyEvent&) {
			PackageTask task;
			task.m_TotalSubTasks = NumScenesToPackage();
			task.m_TaskID = "PackageAudioScenes";
			task.m_TaskName = "Packaging audio scenes";
			task.m_Callback = PackageAudioScenesTask;
			AddPackagingTaskAfter(std::move(task), "PackageScenes");
		});
	}
}
