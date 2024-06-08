#include "SDLAudioExtension.h"
#include "AudioDataEditor.h"

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
	CREATE_OBJECT_CALLBACK_CPP(AudioEmitter, AudioEmitter, ());

	void SDLAudioExtension::HandleStop(Module* pModule)
	{
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		pEngine->GetOptionalModule<SDLAudioModule>()->Stop();
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

		Editor::RegisterEditor<AudioDataEditor>();
		EntitySceneObjectEditor::AddComponentIcon<AudioEmitter>(ICON_FA_VOLUME_HIGH);

		EditorPlayer::RegisterLoopHandler(this);

		OBJECT_CREATE_MENU(AudioEmitter, AudioEmitter);
	}
}
