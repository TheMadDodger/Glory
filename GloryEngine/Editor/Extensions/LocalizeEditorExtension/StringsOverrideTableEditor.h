#pragma once
#include <Editor.h>
#include <EditableResource.h>

#include <StringsOverrideTable.h>

namespace Glory::Editor
{
	class StringsOverrideTableEditor : public EditorTemplate<StringsOverrideTableEditor, YAMLResource<StringsOverrideTable>>
	{
    public:
        StringsOverrideTableEditor();
        virtual ~StringsOverrideTableEditor();

        virtual void Initialize() override;

    private:
        virtual bool OnGUI() override;
        virtual std::string Name() override;
	};
}
