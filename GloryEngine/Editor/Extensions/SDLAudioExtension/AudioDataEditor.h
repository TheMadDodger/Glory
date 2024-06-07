#pragma once
#include <Editor.h>
#include <EditableResource.h>

#include <AudioData.h>

namespace Glory::Editor
{
	class AudioDataEditor : public EditorTemplate<AudioDataEditor, NonEditableResource<AudioData>>
	{
	public:
		AudioDataEditor();
		virtual ~AudioDataEditor() = default;
		virtual bool OnGUI() override;
	};
}
