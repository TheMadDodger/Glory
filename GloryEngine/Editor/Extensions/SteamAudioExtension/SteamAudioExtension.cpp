#include "SteamAudioExtension.h"
#include "AudioSceneWindow.h"
#include "SoundMaterialEditor.h"
#include "SoundOccluderEditor.h"
#include "SoundMaterialImporter.h"

#include <Debug.h>
#include <Engine.h>
#include <AudioComponents.h>
#include <SceneManager.h>
#include <SteamAudioModule.h>
#include <AudioScene.h>
#include <BinaryStream.h>
#include <MeshData.h>
#include <SoundComponents.h>

#include <EntitySceneObjectEditor.h>
#include <CreateEntityObjectsCallbacks.h>
#include <CreateObjectAction.h>
#include <EntityEditor.h>
#include <MainEditor.h>
#include <EditorApplication.h>
#include <MenuBar.h>
#include <Shortcuts.h>

#include <IconsFontAwesome6.h>

EXTENSION_CPP(SteamAudioExtension)

namespace Glory::Editor
{
	SoundMaterialEditor MaterialEditor;
	SoundOccluderEditor OccluderCompEditor;
	SoundMaterialImporter Importer;

	static constexpr char* Shortcut_Window_SteamAudio = "Open Steam Audio Window";

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

	void SteamAudioExtension::Initialize()
	{
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		Reflect::SetReflectInstance(&pEngine->Reflection());

		pEngine->GetSceneManager()->ComponentTypesInstance();

		EditorPlayer::RegisterLoopHandler(this);

		MenuBar::AddMenuItem("Window/Steam Audio", []() { EditorWindow::GetWindow<AudioSceneWindow>(); }, NULL, Shortcut_Window_SteamAudio);

		Editor::RegisterEditor(&MaterialEditor);
		Editor::RegisterEditor(&OccluderCompEditor);
		Importer::Register(&Importer);

		EntitySceneObjectEditor::AddComponentIcon<SoundOccluder>(ICON_FA_EAR_DEAF);
	}
}
