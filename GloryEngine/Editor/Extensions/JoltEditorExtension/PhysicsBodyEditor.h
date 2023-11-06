#pragma once
#include <EntityComponentEditor.h>
#include <Components.h>
#include <Gizmos.h>

namespace Glory::Editor
{
	class PhysicsBodyEditor : public EntityComponentEditor<PhysicsBodyEditor, PhysicsBody>
	{
    public:
        PhysicsBodyEditor();
        virtual ~PhysicsBodyEditor();

        virtual void Initialize() override;

    private:
        virtual bool OnGUI() override;
        virtual std::string Name() override;
	};
}