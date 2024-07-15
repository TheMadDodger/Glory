#include "SDLAudioExtension.h"
#include "AudioDataEditor.h"
#include "AudioSourceEditor.h"

#include <Debug.h>
#include <Engine.h>
#include <AudioComponents.h>
#include <SceneManager.h>
#include <SDLAudioModule.h>

#include <EntitySceneObjectEditor.h>
#include <CreateEntityObjectsCallbacks.h>
#include <CreateObjectAction.h>
#include <EntityEditor.h>
#include <MainEditor.h>
#include <EditorApplication.h>

#include <IconsFontAwesome6.h>

EXTENSION_CPP(SDLAudioExtension)

namespace Glory::Editor
{
	AudioDataEditor AudioEditor;
	AudioSourceEditor SourceEditor;

	CREATE_OBJECT_CALLBACK_CPP(AudioSource, AudioSource, ());
	CREATE_OBJECT_CALLBACK_CPP(AudioListener, AudioListener, ());

	void SDLAudioExtension::HandleStop(Module* pModule)
	{
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		SDLAudioModule* pAudio = pEngine->GetOptionalModule<SDLAudioModule>();
		pAudio->StopAll();
		pAudio->StopMusic();
	}

	SDLAudioExtension::SDLAudioExtension()
	{
	}

	SDLAudioExtension::~SDLAudioExtension()
	{
	}

	void SDLAudioExtension::Initialize()
	{
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		Reflect::SetReflectInstance(&pEngine->Reflection());

		pEngine->GetSceneManager()->ComponentTypesInstance();

		Editor::RegisterEditor(&AudioEditor);
		Editor::RegisterEditor(&SourceEditor);
		EntitySceneObjectEditor::AddComponentIcon<AudioSource>(ICON_FA_VOLUME_HIGH);
		EntitySceneObjectEditor::AddComponentIcon<AudioListener>(ICON_FA_HEADPHONES);

		EditorPlayer::RegisterLoopHandler(this);

		OBJECT_CREATE_MENU(AudioSource, AudioSource);
		OBJECT_CREATE_MENU(AudioListener, AudioListener);
	}
}
