#pragma once
#include <BaseEditorExtension.h>

EXTENSION_H

namespace Glory::Editor
{
	class ASSIMPImporterExtension : public BaseEditorExtension
	{
	public:
		ASSIMPImporterExtension();
		virtual ~ASSIMPImporterExtension();

	private:
		void Initialize() override;
	};
}
