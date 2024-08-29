#include "AudioSceneWindow.h"
#include "AudioSceneTools.h"

#include <PhysicsComponents.h>

#include <Engine.h>
#include <SceneManager.h>
#include <Components.h>

#include <SteamAudioModule.h>
#include <AudioScene.h>
#include <SoundComponents.h>
#include <BinaryStream.h>

#include <EditorApplication.h>

#include <glm/gtx/matrix_decompose.hpp>

#include <phonon/phonon.h>

namespace Glory::Editor
{
	AudioSceneWindow::AudioSceneWindow(): EditorWindowTemplate("Steam Audio", 1600.0f, 600.0f)
	{
	}

	AudioSceneWindow::~AudioSceneWindow()
	{
	}

	void AudioSceneWindow::OnGUI()
	{
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		SteamAudioModule* pModule = pEngine->GetOptionalModule<SteamAudioModule>();
		if (ImGui::Button("Build occlusion scene"))
		{
			SceneManager* sceneManager = pEngine->GetSceneManager();
			for (size_t i = 0; i < sceneManager->OpenScenesCount(); ++i)
			{
				GScene* pScene = sceneManager->GetOpenScene(i);
				
				AudioScene audioScene = GenerateAudioScene(pEngine, pScene, &pModule->DefaultMaterial());

				ProjectSpace* pProject = ProjectSpace::GetOpenProject();
				std::filesystem::path cachedScenesPath = pProject->CachePath();
				cachedScenesPath.append("AudioScenes");
				if (!std::filesystem::exists(cachedScenesPath))
					std::filesystem::create_directory(cachedScenesPath);
				cachedScenesPath.append(std::to_string(pScene->GetUUID())).replace_extension(".gcas");

				BinaryFileStream out{ cachedScenesPath };
				audioScene.Serialize(out);
			}
		}
	}
}
