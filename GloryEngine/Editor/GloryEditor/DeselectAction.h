#pragma once
#include "Undo.h"
#include "Object.h"

namespace Glory::Editor
{
	class DeselectAction : public IAction
	{
    public:
        DeselectAction(UUID deselectedObjectID);
        virtual ~DeselectAction();

        virtual void OnUndo(const ActionRecord& actionRecord);
        virtual void OnRedo(const ActionRecord& actionRecord);

    private:
        UUID m_DeselectedObjectID;
	};
}
