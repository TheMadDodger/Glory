#pragma once
#include "EditorWindow.h"

namespace Glory::Editor
{
	class HistoryWindow : public EditorWindowTemplate<HistoryWindow>
	{
    public:
        HistoryWindow();
        virtual ~HistoryWindow();

    private:
        virtual void OnGUI() override;
	};
}
