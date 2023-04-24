#pragma once
#include "EntityComponentEditor.h"
#include <Components.h>
#include <Gizmos.h>

namespace Glory::Editor
{
	class TransformEditor : public EntityComponentEditor<TransformEditor, Transform>
	{
    public:
        TransformEditor();
        virtual ~TransformEditor();

        virtual void Initialize() override;

    private:
        virtual bool OnGUI() override;
        virtual std::string Name() override;

        void UpdateTransform(const glm::mat4 newTransform);
        void UpdatePhysics();

    private:

        DefaultGizmo* m_pGizmo;
	};
}