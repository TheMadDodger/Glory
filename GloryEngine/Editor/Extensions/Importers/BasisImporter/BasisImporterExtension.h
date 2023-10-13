#pragma once
#include <BaseEditorExtension.h>

EXTENSION_H

namespace Glory::Editor
{
    class BasisImporterExtension : public BaseEditorExtension
    {
    public:
        BasisImporterExtension();
        virtual ~BasisImporterExtension();

    private:
        virtual void Initialize() override;
    };
}
