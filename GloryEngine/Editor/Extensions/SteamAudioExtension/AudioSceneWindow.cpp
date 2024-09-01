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
#include <EditorSceneManager.h>
#include <Dispatcher.h>
#include <EditorUI.h>

#include <glm/gtx/matrix_decompose.hpp>

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
		EditorSceneManager& sceneManager = EditorApplication::GetInstance()->GetSceneManager();
		SteamAudioModule* pModule = pEngine->GetOptionalModule<SteamAudioModule>();
		const size_t sceneCount = sceneManager.OpenScenesCount();

		ImGui::BeginDisabled(sceneCount == 0);
		if (ImGui::Button("Build occlusion scene"))
		{
			for (size_t i = 0; i < sceneManager.OpenScenesCount(); ++i)
			{
				GScene* pScene = sceneManager.GetOpenScene(i);
				
				AudioScene audioScene{ pScene->GetUUID() };
				if (!GenerateAudioScene(pEngine, pScene, &pModule->DefaultMaterial(), audioScene))
					continue;

				ProjectSpace* pProject = ProjectSpace::GetOpenProject();
				std::filesystem::path cachedScenesPath = pProject->CachePath();
				cachedScenesPath.append("AudioScenes");
				if (!std::filesystem::exists(cachedScenesPath))
					std::filesystem::create_directory(cachedScenesPath);
				cachedScenesPath.append(std::to_string(pScene->GetUUID())).replace_extension(".gcas");

				BinaryFileStream out{ cachedScenesPath };
				audioScene.Serialize(out);
			}

			m_AudioScenes.clear();
			LoadAllCachedScenes(sceneManager);
		}
		ImGui::EndDisabled();

		if (sceneCount == 0)
		{
			ImGui::TextUnformatted("No scenes iopen");
			return;
		}

		for (size_t i = 0; i < sceneCount; ++i)
		{
			GScene* pScene = sceneManager.GetOpenScene(i);
			ImGui::PushID(pScene->Name().data());
			if (EditorUI::Header(pScene->Name()))
			{
				ImGui::Indent(5.0f);
				auto iter = std::find_if(m_AudioScenes.begin(), m_AudioScenes.end(), [pScene](AudioScene& scene) { return scene.SceneID() == pScene->GetUUID(); });
				if (iter == m_AudioScenes.end())
				{
					ImGui::TextUnformatted("No audio scene generated for this scene");
					ImGui::Unindent(5.0f);
					ImGui::PopID();
					continue;
				}

				EditorUI::LabelText("Mesh count", std::to_string(iter->MeshCount()));
				ImGui::PushID("Meshes");
				if (EditorUI::Header("Meshes"))
				{
					ImGui::Indent(5.0f);
					for (size_t i = 0; i < iter->MeshCount(); ++i)
					{
						MeshData mesh = iter->Mesh(i);

						ImGui::PushID(i);
						EditorUI::LabelText("Mesh " + std::to_string(i), std::to_string(mesh.VertexCount()));
						ImGui::PopID();
					}
					ImGui::Unindent(5.0f);
				}
				ImGui::PopID();
				ImGui::Unindent(5.0f);
			}
			ImGui::PopID();
		}
	}

	void AudioSceneWindow::OnOpen()
	{
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		EditorSceneManager& sceneManager = EditorApplication::GetInstance()->GetSceneManager();

		m_SceneEventsListener = sceneManager.SceneEventsDispatcher().AddListener([pEngine, &sceneManager, this](const EditorSceneManager::EditorSceneEvent& e) {
			switch (e.Type)
			{
			case EditorSceneManager::Opened:
			{
				ReloadSceneCache(sceneManager, e.SceneID);
				break;
			}
			default:
				break;
			}
		});

		LoadAllCachedScenes(sceneManager);
	}

	void AudioSceneWindow::OnClose()
	{
		EditorSceneManager& sceneManager = EditorApplication::GetInstance()->GetSceneManager();
		sceneManager.SceneEventsDispatcher().RemoveListener(m_SceneEventsListener);
		m_AudioScenes.clear();
	}

	void AudioSceneWindow::LoadAllCachedScenes(EditorSceneManager& sceneManager)
	{
		ProjectSpace* pProject = ProjectSpace::GetOpenProject();
		std::filesystem::path cachedScenesPath = pProject->CachePath();
		cachedScenesPath.append("AudioScenes");
		if (!std::filesystem::exists(cachedScenesPath)) return;

		for (size_t i = 0; i < sceneManager.OpenScenesCount(); ++i)
		{
			GScene* pScene = sceneManager.GetOpenScene(i);
			std::filesystem::path cachedScenePath = cachedScenesPath;
			cachedScenePath.append(std::to_string(pScene->GetUUID())).replace_extension(".gcas");
			if (!std::filesystem::exists(cachedScenePath)) continue;
			BinaryFileStream fileStream{ cachedScenePath, true };
			AudioScene audioScene{ pScene->GetUUID() };
			audioScene.Deserialize(fileStream);
			m_AudioScenes.push_back(std::move(audioScene));
		}
	}

	void AudioSceneWindow::ReloadSceneCache(EditorSceneManager& sceneManager, UUID sceneID)
	{
		ProjectSpace* pProject = ProjectSpace::GetOpenProject();
		std::filesystem::path cachedScenesPath = pProject->CachePath();
		cachedScenesPath.append("AudioScenes");
		if (!std::filesystem::exists(cachedScenesPath)) return;

		std::filesystem::path cachedScenePath = cachedScenesPath;
		cachedScenePath.append(std::to_string(sceneID)).replace_extension(".gcas");
		if (!std::filesystem::exists(cachedScenePath)) return;

		BinaryFileStream fileStream{ cachedScenePath, true };

		auto iter = std::find_if(m_AudioScenes.begin(), m_AudioScenes.end(), [sceneID](AudioScene& scene) { return scene.SceneID() == sceneID; });
		if (iter == m_AudioScenes.end())
		{
			m_AudioScenes.push_back(AudioScene(sceneID));
			iter = m_AudioScenes.end();
			--iter;
		}

		iter->Deserialize(fileStream);
	}
}
