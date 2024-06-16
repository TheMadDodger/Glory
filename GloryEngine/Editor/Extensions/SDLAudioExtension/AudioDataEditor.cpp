#include "AudioDataEditor.h"

#include <SDLAudioModule.h>
#include <AssetManager.h>
#include <Debug.h>

#include <EditorUI.h>
#include <EditorApplication.h>

#include <IconsFontAwesome6.h>
#include <EditorAssetDatabase.h>

namespace Glory::Editor
{
	AudioDataEditor::AudioDataEditor()
	{
	}

	bool AudioDataEditor::OnGUI()
	{
		NonEditableResource<AudioData>* pAudioData = (NonEditableResource<AudioData>*)m_pTarget;
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		SDLAudioModule* pAudioModule = pEngine->GetOptionalModule<SDLAudioModule>();

		const std::string name = EditorAssetDatabase::GetAssetName(pAudioData->GetUUID());
		ImGui::Text("Audio file: %s", name.data());
		ImGui::Separator();
		ImGui::TextUnformatted("Preview");

		const float buttonSize = 32.0f;
		if (ImGui::Button(ICON_FA_PLAY, { buttonSize, buttonSize }))
		{
			Resource* pResource = pEngine->GetAssetManager().FindResource(pAudioData->GetUUID());
			if (!pResource)
			{
				pEngine->GetDebug().LogError("Could not preview audio file because it is not yet loaded.");
				return false;
			}

			if (m_PlayingChannel != -1 && pAudioModule->IsPlaying(m_PlayingChannel))
				pAudioModule->Stop(m_PlayingChannel);
			m_PlayingChannel = pAudioModule->Play(static_cast<AudioData*>(pResource));
		}

		ImGui::SameLine();
		ImGui::BeginDisabled(!(m_PlayingChannel != -1 && pAudioModule->IsPlaying(m_PlayingChannel)));
		if (ImGui::Button(ICON_FA_STOP, { buttonSize, buttonSize }))
		{
			pAudioModule->Stop(m_PlayingChannel);
		}
		ImGui::EndDisabled();

		return false;
	}
}
