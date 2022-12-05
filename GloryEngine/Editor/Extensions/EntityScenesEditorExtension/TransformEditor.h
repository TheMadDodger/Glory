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

        void UpdateTransform();

    private:
        glm::mat4 m_Transform;
        glm::mat4 m_LastTransform;

        DefaultGizmo* m_pGizmo;
	};
}