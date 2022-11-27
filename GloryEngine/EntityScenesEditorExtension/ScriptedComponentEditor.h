#pragma once
#include "EntityComponentEditor.h"
#include <Components.h>
#include <Gizmos.h>

namespace Glory
{
    class Script;
}

namespace Glory::Editor
{
	class ScriptedComponentEditor : public EntityComponentEditor<ScriptedComponentEditor, ScriptedComponent>
	{
    public:
        ScriptedComponentEditor();
        virtual ~ScriptedComponentEditor();

        virtual void Initialize() override;

    private:
        virtual bool OnGUI() override;
        virtual std::string Name() override;

    private:
        Script* m_pScript;
	};
}