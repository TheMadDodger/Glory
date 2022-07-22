#pragma once
#include <vector>
#include "Editor.h"
#include "EditorContext.h"

namespace Glory::Editor
{
	class BaseEditorExtension
	{
	public:
		BaseEditorExtension();
		virtual ~BaseEditorExtension();

	protected:
		virtual void RegisterEditors() = 0;

	private:
		friend class EditorApplication;
	};
}
