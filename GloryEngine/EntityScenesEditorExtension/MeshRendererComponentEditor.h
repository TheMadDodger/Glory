#pragma once
#include "EntityComponentEditor.h"
#include <Components.h>

namespace Glory::Editor
{
	class MeshRendererComponentEditor : public EntityComponentEditor<MeshRendererComponentEditor, MeshRenderer>
	{
	public:
        MeshRendererComponentEditor();
        virtual ~MeshRendererComponentEditor();

    private:
        virtual void OnGUI() override;
        virtual std::string Name() override;
	};
}
