#include "SDLAudioExtension.h"
#include "AudioDataEditor.h"

#include <Importer.h>

EXTENSION_CPP(SDLAudioExtension)

namespace Glory::Editor
{
	SDLAudioExtension::SDLAudioExtension()
	{
	}

	SDLAudioExtension::~SDLAudioExtension()
	{
	}

	void SDLAudioExtension::Initialize()
	{
		Editor::RegisterEditor<AudioDataEditor>();
	}
}
