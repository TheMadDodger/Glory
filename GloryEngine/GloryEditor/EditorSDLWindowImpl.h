#pragma once
#include "EditorWindowImpl.h"

namespace Glory::Editor
{
	class EditorSDLWindowImpl : public EditorWindowImpl
	{
	public:
		EditorSDLWindowImpl();
		virtual ~EditorSDLWindowImpl();

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
