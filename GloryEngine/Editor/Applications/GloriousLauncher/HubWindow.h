#pragma once

namespace Glory::EditorLauncher
{
	class HubWindow
	{
	public:
		HubWindow(const std::string& windowName, const std::string& glslVersion = "#version 130");
		virtual ~HubWindow();

		bool Initialize();

		SDL_Window* GetSDLWindow();
		SDL_GLContext GetGLContext();
		const char* GetGLSLVersion();

		bool PollEvents();
		void Swap();

	private:
		const std::string m_WindowName;
		const std::string m_GLSLVersion;
		SDL_Window* m_pWindow;
		SDL_GLContext m_GLContext;
	};
}
