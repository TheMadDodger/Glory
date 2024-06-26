#include "SteamAudioExtension.h"
#include "AudioSceneWindow.h"

#include <Debug.h>
#include <Engine.h>
#include <AudioComponents.h>
#include <SceneManager.h>
#include <SteamAudioModule.h>

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
	static constexpr char* Shortcut_Window_SteamAudio = "Open Steam Audio Window";

	CREATE_OBJECT_CALLBACK_CPP(AudioSource, AudioSource, ());
	CREATE_OBJECT_CALLBACK_CPP(AudioListener, AudioListener, ());

	void SteamAudioExtension::HandleStop(Module* pModule)
	{
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		SteamAudioModule* pAudio = pEngine->GetOptionalModule<SteamAudioModule>();
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

		//Editor::RegisterEditor<AudioDataEditor>();
		//EntitySceneObjectEditor::AddComponentIcon<AudioSource>(ICON_FA_VOLUME_HIGH);
		//EntitySceneObjectEditor::AddComponentIcon<AudioListener>(ICON_FA_HEADPHONES);

		EditorPlayer::RegisterLoopHandler(this);

		//OBJECT_CREATE_MENU(AudioSource, AudioSource);
		//OBJECT_CREATE_MENU(AudioListener, AudioListener);

		MenuBar::AddMenuItem("Window/Steam Audio", []() { EditorWindow::GetWindow<AudioSceneWindow>(); }, NULL, Shortcut_Window_SteamAudio);
	}
}
