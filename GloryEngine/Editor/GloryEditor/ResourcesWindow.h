#pragma once
#include "EditorWindow.h"

namespace Glory::Editor
{
	class ResourcesWindow : public EditorWindowTemplate<ResourcesWindow>
	{
    public:
        ResourcesWindow();
        virtual ~ResourcesWindow();

    private:
        virtual void OnGUI() override;
	};
}
