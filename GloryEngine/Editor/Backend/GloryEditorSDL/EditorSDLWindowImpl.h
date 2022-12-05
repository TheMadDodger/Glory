#pragma once
#include "EditorWindowImpl.h"
#include <Glory.h>
#include <EditorCreateInfo.h>

namespace Glory::Editor
{
	class EditorApplication;

	extern "C" GLORY_API void LoadBackend(EditorCreateInfo& editorCreateInfo);

	class EditorSDLWindowImpl : public EditorWindowImpl
	{
	public:
		EditorSDLWindowImpl();
		virtual ~EditorSDLWindowImpl();

		virtual void SetContext(ImGuiContext* pImguiConext) override;

	protected:
		virtual void Shutdown() override;

		virtual void SetupForOpenGL() override;
		virtual void SetupForVulkan() override;
		virtual void SetupForD3D() override;
		virtual void SetupForMetal() override;

		virtual bool PollEvents() override;
		virtual void NewFrame() override;
	};
}
