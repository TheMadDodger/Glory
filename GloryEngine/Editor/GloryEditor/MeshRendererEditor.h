#pragma once
#include <EntityComponentEditor.h>
#include <Components.h>
#include <Gizmos.h>

namespace Glory::Editor
{
	class MeshRendererEditor : public EntityComponentEditor<MeshRendererEditor, MeshRenderer>
	{
    public:
        MeshRendererEditor();
        virtual ~MeshRendererEditor();

        virtual void Initialize() override;

    private:
        virtual bool OnGUI() override;
        virtual std::string Name() override;
	};
}