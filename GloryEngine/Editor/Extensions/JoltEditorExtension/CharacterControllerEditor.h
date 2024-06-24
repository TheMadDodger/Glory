#pragma once
#include "EntityComponentEditor.h"
#include <Gizmos.h>
#include <PhysicsComponents.h>

namespace Glory::Editor
{
	class CharacterControllerEditor : public EntityComponentEditor<CharacterControllerEditor, CharacterController>
	{
    public:
        CharacterControllerEditor();
        virtual ~CharacterControllerEditor();

        virtual void Initialize() override;

    private:
        virtual bool OnGUI() override;
        virtual std::string Name() override;
	};
}