#pragma once
#include "EntityComponentEditor.h"

#include <Components.h>

namespace Glory::Editor
{
	class TextComponentEditor : public EntityComponentEditor<TextComponentEditor, TextComponent>
    {
    public:
        TextComponentEditor();
        virtual ~TextComponentEditor();

        virtual void Initialize() override;

    private:
        virtual bool OnGUI() override;
        virtual std::string Name() override;
	};
}
