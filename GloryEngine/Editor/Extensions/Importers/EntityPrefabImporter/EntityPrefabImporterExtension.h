#pragma once
#include <BaseEditorExtension.h>

EXTENSION_H

namespace Glory::Editor
{
    class EntityPrefabImporterExtension : public BaseEditorExtension
    {
    public:
        EntityPrefabImporterExtension();
        virtual ~EntityPrefabImporterExtension();

    private:
        virtual void Initialize() override;
    };
}
