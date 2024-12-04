#include "SDLWindow.h"
#include "SDLWindowExceptions.h"
#include "WindowModule.h"

#include <Input.h>
#include <Engine.h>
#include <Debug.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_vulkan.h>
#include <iostream>
#include <map>

namespace Glory
{
	const std::map<SDL_Keycode, KeyboardKey> KEYBOARD_KEYMAP = {
		{SDLK_ESCAPE,			   KeyEscape,					   },
		{SDLK_F1,				   KeyF1,						   },
		{SDLK_F2,				   KeyF2,						   },
		{SDLK_F3,				   KeyF3,						   },
		{SDLK_F4,				   KeyF4,						   },
		{SDLK_F5,				   KeyF5,						   },
		{SDLK_F6,				   KeyF6,						   },
		{SDLK_F7,				   KeyF7,						   },
		{SDLK_F8,				   KeyF8,						   },
		{SDLK_F9,				   KeyF9,						   },
		{SDLK_F10,			   KeyF10,						   },
		{SDLK_F11,			   KeyF11,						   },
		{SDLK_F12,			   KeyF12,						   },
		{SDLK_F13,			   KeyF13,						   },
		{SDLK_F14,			   KeyF14,						   },
		{SDLK_F15,			   KeyF15,						   },
		{SDLK_F16,			   KeyF16,						   },
		{SDLK_F17,			   KeyF17,						   },
		{SDLK_F18,			   KeyF18,						   },
		{SDLK_F19,			   KeyF19,						   },
		{SDLK_PRINTSCREEN,			   KeyPrint,					   },
		{SDLK_SCROLLLOCK,		   KeyScrollLock,				   },
		//{SDLK_BREAK,			   KeyBreak,					   },

		{SDLK_SPACE,			   KeySpace,					   },

		//{SDLK_APOSTROPHE,		   KeyApostrophe,				   },
		{SDLK_COMMA,			   KeyComma,					   },
		{SDLK_MINUS,			   KeyMinus,					   },
		{SDLK_PERIOD,			   KeyPeriod,					   },
		{SDLK_SLASH,			   KeySlash,					   },

		{SDLK_0,				   Key0,						   },
		{SDLK_1,				   Key1,						   },
		{SDLK_2,				   Key2,						   },
		{SDLK_3,				   Key3,						   },
		{SDLK_4,				   Key4,						   },
		{SDLK_5,				   Key5,						   },
		{SDLK_6,				   Key6,						   },
		{SDLK_7,				   Key7,						   },
		{SDLK_8,				   Key8,						   },
		{SDLK_9,				   Key9,						   },

		{SDLK_SEMICOLON,		   KeySemicolon,				   },
		{SDLK_LESS,					KeyLess,						   },
		{SDLK_EQUALS,			   KeyEqual,					   },

		{SDLK_a,				   KeyA,						   },
		{SDLK_b,				   KeyB,						   },
		{SDLK_c,				   KeyC,						   },
		{SDLK_d,				   KeyD,						   },
		{SDLK_e,				   KeyE,						   },
		{SDLK_f,				   KeyF,						   },
		{SDLK_g,				   KeyG,						   },
		{SDLK_h,				   KeyH,						   },
		{SDLK_i,				   KeyI,						   },
		{SDLK_j,				   KeyJ,						   },
		{SDLK_k,				   KeyK,						   },
		{SDLK_l,				   KeyL,						   },
		{SDLK_m,				   KeyM,						   },
		{SDLK_n,				   KeyN,						   },
		{SDLK_o,				   KeyO,						   },
		{SDLK_p,				   KeyP,						   },
		{SDLK_q,				   KeyQ,						   },
		{SDLK_r,				   KeyR,						   },
		{SDLK_s,				   KeyS,						   },
		{SDLK_t,				   KeyT,						   },
		{SDLK_u,				   KeyU,						   },
		{SDLK_v,				   KeyV,						   },
		{SDLK_w,				   KeyW,						   },
		{SDLK_x,				   KeyX,						   },
		{SDLK_y,				   KeyY,						   },
		{SDLK_z,				   KeyZ,						   },

		{SDLK_LEFTBRACKET,	   KeyBracketLeft,				   },
		{SDLK_BACKSLASH,		   KeyBackslash,				   },
		{SDLK_RIGHTBRACKET,	   KeyBracketRight,				   },

		//{SDLK_GRAVE,			   KeyGrave,					   },

		{SDLK_LEFT,			   KeyLeft,						   },
		{SDLK_RIGHT,			   KeyRight,					   },
		{SDLK_UP,				   KeyUp,						   },
		{SDLK_DOWN,			   KeyDown,						   },
		{SDLK_INSERT,			   KeyInsert,					   },
		{SDLK_HOME,			   KeyHome,						   },
		{SDLK_DELETE,			   KeyDelete,					   },
		{SDLK_END,			   KeyEnd,						   },
		{SDLK_PAGEUP,			   KeyPageUp,					   },
		{SDLK_PAGEDOWN,		   KeyPageDown,					   },

		{SDLK_NUMLOCKCLEAR,		   KeyNumLock,					   },
		{SDLK_KP_EQUALS,		   KeyKpEqual,					   },
		{SDLK_KP_DIVIDE,		   KeyKpDivide,					   },
		{SDLK_KP_MULTIPLY,		   KeyKpMultiply,				   },
		{SDLK_KP_MEMSUBTRACT,		   KeyKpSubtract,				   },
		{SDLK_KP_MEMADD,			   KeyKpAdd,					   },
		{SDLK_KP_ENTER,		   KeyKpEnter,					   },
		//{SDLK_KP_INSERT,		   KeyKpInsert,					   },
		//{SDLK_KP_END,			   KeyKpEnd,					   },
		//{SDLK_KP_DOWN,			   KeyKpDown,					   },
		//{SDLK_KpPageDown,		   KeyKpPageDown,				   },
		//{SDLK_KpLeft,			   KeyKpLeft,					   },
		//{SDLK_KpBegin,		   KeyKpBegin,					   },
		//{SDLK_KpRight,		   KeyKpRight,					   },
		//{SDLK_KpHome,			   KeyKpHome,					   },
		//{SDLK_KpUp,			   KeyKpUp,						   },
		//{SDLK_KpPageUp,		   KeyKpPageUp,					   },
		//{SDLK_KpDelete,		   KeyKpDelete,					   },

		{SDLK_BACKSPACE,		   KeyBackSpace,				   },
		{SDLK_TAB,			   KeyTab,						   },
		{SDLK_RETURN,			   KeyReturn,					   },
		{SDLK_CAPSLOCK,		   KeyCapsLock,					   },
		{SDLK_LSHIFT,			   KeyShiftL,					   },
		{SDLK_LCTRL,			   KeyCtrlL,					   },
		//{SDLK_SUPER,			   KeySuperL,					   },
		{SDLK_LALT,			   KeyAltL,						   },
		{SDLK_RALT,			   KeyAltR,						   },
		//{SDLK_SuperR,			   KeySuperR,					   },
		{SDLK_MENU,			   KeyMenu,						   },
		{SDLK_RCTRL,			   KeyCtrlR,					   },
		{SDLK_RSHIFT,			   KeyShiftR,					   },

		//{SDLK_BACK,			   KeyBack,						   },
		//{SDLK_SoftLeft,		   KeySoftLeft,					   },
		//{SDLK_SoftRight,		   KeySoftRight,				   },
		//{SDLK_Call,			   KeyCall,						   },
		//{SDLK_Endcall,		   KeyEndcall,					   },
		//{SDLK_Star,			   KeyStar,						   },
		//{SDLK_Pound,			   KeyPound,					   },
		//{SDLK_DpadCenter,		   KeyDpadCenter,				   },
		{SDLK_VOLUMEUP,		   KeyVolumeUp,					   },
		{SDLK_VOLUMEDOWN,		   KeyVolumeDown,				   },
		{SDLK_POWER,			   KeyPower,					   },
		//{SDLK_Camera,			   KeyCamera,					   },
		{SDLK_CLEAR,			   KeyClear,					   },
		//{SDLK_Symbol,			   KeySymbol,					   },
		//{SDLK_Explorer,		   KeyExplorer,					   },
		//{SDLK_Envelope,		   KeyEnvelope,					   },
		{SDLK_EQUALS,			   KeyEquals,					   },
		{SDLK_AT,				   KeyAt,						   },
		//{SDLK_Headsethook,	   KeyHeadsethook,				   },
		//{SDLK_Focus,			   KeyFocus,					   },
		{SDLK_PLUS,			   KeyPlus,						   },
		//{SDLK_Notification,	   KeyNotification,				   },
		//{SDLK_Search,			   KeySearch,					   },
		//{SDLK_MediaPlayPause,	   KeyMediaPlayPause,			   },
		//{SDLK_MediaStop,		   KeyMediaStop,				   },
		//{SDLK_MediaNext,		   KeyMediaNext,				   },
		//{SDLK_MediaPrevious,	   KeyMediaPrevious,			   },
		//{SDLK_MediaRewind,	   KeyMediaRewind,				   },
		//{SDLK_MediaFastForward,  KeyMediaFastForward,			   },
		//{SDLK_Mute,			   KeyMute,						   },
		//{SDLK_Pictsymbols,	   KeyPictsymbols,				   },
		//{SDLK_SwitchCharset,	   KeySwitchCharset,			   },

		{SDL_SCANCODE_AC_FORWARD,		   KeyForward,					   },
		//{SDLK_Extra1,			   KeyExtra1,					   },
		//{SDLK_Extra2,			   KeyExtra2,					   },
		//{SDLK_Extra3,			   KeyExtra3,					   },
		//{SDLK_Extra4,			   KeyExtra4,					   },
		//{SDLK_Extra5,			   KeyExtra5,					   },
		//{SDLK_Extra6,			   KeyExtra6,					   },
		//{SDLK_Fn,				   KeyFn,						   },

		//{SDLK_Circumflex,		   KeyCircumflex,				   },
		//{SDLK_Ssharp,			   KeySsharp,					   },
		//{SDLK_Acute,			   KeyAcute,					   },
		{SDLK_ALTERASE,			   KeyAltGr,					   },
		//{SDLK_Numbersign,		   KeyNumbersign,				   },
		//{SDLK_Udiaeresis,		   KeyUdiaeresis,				   },
		//{SDLK_Adiaeresis,		   KeyAdiaeresis,				   },
		//{SDLK_Odiaeresis,		   KeyOdiaeresis,				   },
		//{SDLK_Section,		   KeySection,					   },
		//{SDLK_Aring,			   KeyAring,					   },
		//{SDLK_Diaeresis,		   KeyDiaeresis,				   },
		//{SDLK_Twosuperior,	   KeyTwosuperior,				   },
		//{SDLK_RightParenthesis,  KeyRightParenthesis,			   },
		{SDLK_DOLLAR,			   KeyDollar,					   },
		//{SDLK_Ugrave,			   KeyUgrave,					   },
		{SDLK_ASTERISK,		   KeyAsterisk,							},
		{SDLK_COLON,			   KeyColon,					   },
		{SDLK_EXCLAIM,			   KeyExclam,					   },

		{SDLK_KP_LEFTBRACE,		   KeyBraceLeft,				   },
		{SDLK_KP_RIGHTBRACE,		   KeyBraceRight,				},
		{SDLK_SYSREQ,			   KeySysRq,					   },
	};
	const std::map<uint8_t, MouseButton> MOUSE_BUTTONMAP = {
		{ SDL_BUTTON_LEFT, MouseButton::MouseButtonLeft },
		{ SDL_BUTTON_RIGHT, MouseButton::MouseButtonRight },
		{ SDL_BUTTON_MIDDLE, MouseButton::MouseButtonMiddle },
		{ SDL_BUTTON_X1, MouseButton::MouseButtonSide1 },
		{ SDL_BUTTON_X2, MouseButton::MouseButtonSide2 },
	};

	void SDLWindow::GetVulkanSurface(void* instance, void* surface)
	{
		if (!SDL_Vulkan_CreateSurface(m_pWindow, static_cast<VkInstance>(instance), (VkSurfaceKHR*)surface))
		{
			const char* error = SDL_GetError();
			std::cout << "Could not create a Vulkan surface. ERROR: " << error << std::endl;
			throw new SDLErrorException(error);
		}
	}

	void SDLWindow::GetDrawableSize(int* width, int* height)
	{
		SDL_GL_GetDrawableSize(m_pWindow, width, height);
	}

	void SDLWindow::GetWindowSize(int* width, int* height)
	{
		SDL_GetWindowSize(m_pWindow, width, height);
	}

	void SDLWindow::GetWindowPosition(int* x, int* y)
	{
		SDL_GetWindowPosition(m_pWindow, x, y);
	}

	void SDLWindow::SetupForOpenGL()
	{
		// Create OpenGL context
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
		m_GLSDLContext = SDL_GL_CreateContext(m_pWindow);
		if (m_GLSDLContext == nullptr)
		{
			std::cerr << "Could not ceate SDL GL Context: " << SDL_GetError() << std::endl;
			return;
		}

		if (SDL_GL_SetSwapInterval(0) < 0)
		{
			std::cerr << "Could not set SDL GL Swap interval: " << SDL_GetError() << std::endl;
			return;
		}
	}

	void SDLWindow::CleanupOpenGL()
	{
		SDL_GL_DeleteContext(m_GLSDLContext);
		m_GLSDLContext = nullptr;
	}

	void SDLWindow::GLSwapWindow()
	{
		SDL_GL_SwapWindow(m_pWindow);
	}

	void SDLWindow::MakeGLContextCurrent()
	{
		SDL_GL_MakeCurrent(m_pWindow, m_GLSDLContext);
	}

	SDL_Window* SDLWindow::GetSDLWindow()
	{
		return m_pWindow;
	}

	SDL_GLContext SDLWindow::GetSDLGLConext() const
	{
		return m_GLSDLContext;
	}

	bool SDLWindow::PollEvent(SDL_Event* event)
	{
		return SDL_PollEvent(event);
	}

	bool SDLWindow::HandleInputEvents(SDL_Event& event)
	{
		InputEvent inputEvent;
		switch (event.type)
		{
		case SDL_KEYDOWN:
		{
			SDL_Keycode keycode = event.key.keysym.sym;
			const auto iter = KEYBOARD_KEYMAP.find(keycode);
			if (iter == KEYBOARD_KEYMAP.end()) break;
			inputEvent.InputDeviceType = InputDeviceType::Keyboard;
			inputEvent.KeyID = iter->second;
			inputEvent.State = InputState::KeyDown;
			inputEvent.SourceDeviceID = 0;
			inputEvent.Value = 1.0f;
			inputEvent.Delta = 1.0f;
			return ForwardInputEvent(inputEvent);
		}
		case SDL_KEYUP:
		{
			SDL_Keycode keycode = event.key.keysym.sym;
			const auto iter = KEYBOARD_KEYMAP.find(keycode);
			if (iter == KEYBOARD_KEYMAP.end()) break;
			inputEvent.InputDeviceType = InputDeviceType::Keyboard;
			inputEvent.KeyID = iter->second;
			inputEvent.State = InputState::KeyUp;
			inputEvent.SourceDeviceID = 0;
			inputEvent.Value = 0.0f;
			inputEvent.Delta = -1.0f;
			return ForwardInputEvent(inputEvent);
		}
		case SDL_MOUSEBUTTONDOWN:
		{
			inputEvent.InputDeviceType = InputDeviceType::Mouse;
			inputEvent.State = InputState::KeyDown;

			const auto iter = MOUSE_BUTTONMAP.find(event.button.button);
			if (iter == MOUSE_BUTTONMAP.end())
				inputEvent.KeyID = event.button.button - 1;
			else
				inputEvent.KeyID = iter->second;

			inputEvent.SourceDeviceID = event.button.which;
			inputEvent.Value = 1.0f;
			inputEvent.Delta = 1.0f;
			return ForwardInputEvent(inputEvent);
		}
		case SDL_MOUSEBUTTONUP:
		{
			inputEvent.InputDeviceType = InputDeviceType::Mouse;
			inputEvent.State = InputState::KeyUp;

			const auto iter = MOUSE_BUTTONMAP.find(event.button.button);
			if (iter == MOUSE_BUTTONMAP.end())
				inputEvent.KeyID = event.button.button - 1;
			else
				inputEvent.KeyID = iter->second;

			inputEvent.SourceDeviceID = event.button.which;
			inputEvent.Value = 0.0f;
			inputEvent.Delta = -1.0f;
			return ForwardInputEvent(inputEvent);
		}
		case SDL_MOUSEWHEEL:
		{
			bool consumed = false;
			if (event.wheel.x != 0)
			{
				/* Axis event */
				inputEvent.InputDeviceType = InputDeviceType::Mouse;
				inputEvent.KeyID = MouseAxis::MouseAxisScrollX;
				inputEvent.State = InputState::Axis;
				inputEvent.SourceDeviceID = event.wheel.which;
				inputEvent.Value = (float)event.wheel.x;
				inputEvent.Delta = (float)event.wheel.x;
				consumed |= ForwardInputEvent(inputEvent);
			}

			if (event.wheel.y != 0)
			{
				/* Axis event */
				inputEvent.InputDeviceType = InputDeviceType::Mouse;
				inputEvent.KeyID = MouseAxis::MouseAxisScrollY;
				inputEvent.State = InputState::Axis;
				inputEvent.SourceDeviceID = event.wheel.which;
				inputEvent.Value = (float)event.wheel.y;
				inputEvent.Delta = (float)event.wheel.y;
				consumed |= ForwardInputEvent(inputEvent);

				/* Button event */
				inputEvent.InputDeviceType = InputDeviceType::Mouse;
				inputEvent.KeyID = event.wheel.y < 0 ? MouseButton::MouseButtonWheelDown : MouseButton::MouseButtonWheelUp;
				inputEvent.State = InputState::KeyDown;
				inputEvent.SourceDeviceID = event.wheel.which;
				inputEvent.Value = 1.0f;
				inputEvent.Delta = 1.0f;
				consumed |= ForwardInputEvent(inputEvent);
				inputEvent.State = InputState::KeyUp;
				inputEvent.Value = 0.0f;
				inputEvent.Delta = -1.0f;
				consumed |= ForwardInputEvent(inputEvent);
			}
			return consumed;
		}
		case SDL_MOUSEMOTION:
		{
			bool consumed = false;
			inputEvent.InputDeviceType = InputDeviceType::Mouse;
			inputEvent.KeyID = MouseAxis::MouseAxisX;
			inputEvent.State = InputState::Axis;
			inputEvent.SourceDeviceID = event.motion.which;
			inputEvent.Value = (float)event.motion.x;
			inputEvent.Delta = (float)event.motion.xrel;
			consumed |= ForwardInputEvent(inputEvent);

			inputEvent.InputDeviceType = InputDeviceType::Mouse;
			inputEvent.KeyID = MouseAxis::MouseAxisY;
			inputEvent.State = InputState::Axis;
			inputEvent.SourceDeviceID = event.motion.which;
			inputEvent.Value = (float)event.motion.y;
			inputEvent.Delta = (float)event.motion.yrel;
			consumed |= ForwardInputEvent(inputEvent);
			return consumed;
		}
		default:
			// Do nothing.
			break;
		}

		return false;
	}

	void SDLWindow::Resize(int width, int height)
	{
		m_Width = width;
		m_Height = height;
		SDL_SetWindowSize(m_pWindow, width, height);
	}

	void SDLWindow::GetPosition(int* width, int* height)
	{
		SDL_GetWindowPosition(m_pWindow, width, height);
	}

	void SDLWindow::SetPosition(int width, int height)
	{
		SDL_SetWindowPosition(m_pWindow, width, height);
	}

	void SDLWindow::SetWindowTitle(const char* title)
	{
		SDL_SetWindowTitle(m_pWindow, title);
	}

	void SDLWindow::GetWindowBordersSize(int* top, int* left, int* bottom, int* right)
	{
		if (SDL_GetWindowBordersSize(m_pWindow, top, left, bottom, right) == 0) return;
		m_pWindowManager->GetEngine()->GetDebug().LogError(SDL_GetError());
	}

	void SDLWindow::SetSplashScreen(const char* data, size_t size)
	{
		if (m_pSplashScreen)
		{
			SDL_FreeSurface(m_pSplashScreen);
			m_pSplashScreen = nullptr;
		}

		SDL_RWops* pPixels = SDL_RWFromConstMem(data, size);
		m_pSplashScreen = SDL_LoadBMP_RW(pPixels, 1);

		if (!m_pSplashScreen)
		{
			m_pWindowManager->GetEngine()->GetDebug().LogFatalError("Failed to load splash screen!");
			return;
		}

		SDL_BlitSurface(m_pSplashScreen, NULL, m_pWindowSurface, NULL);
		SDL_UpdateWindowSurface(m_pWindow);
	}

	void SDLWindow::SetSplashScreen(const std::filesystem::path& path)
	{
		if (m_pSplashScreen)
		{
			SDL_FreeSurface(m_pSplashScreen);
			m_pSplashScreen = nullptr;
		}

		m_pSplashScreen = SDL_LoadBMP(path.string().data());

		if (!m_pSplashScreen)
		{
			m_pWindowManager->GetEngine()->GetDebug().LogFatalError("Failed to load splash screen!");
			return;
		}

		SDL_BlitSurface(m_pSplashScreen, NULL, m_pWindowSurface, NULL);
		SDL_UpdateWindowSurface(m_pWindow);
	}

	SDLWindow::SDLWindow(const WindowCreateInfo& createInfo) : Window(createInfo),
		m_pWindow(nullptr), m_GLSDLContext(NULL), m_pWindowSurface(nullptr), m_pSplashScreen(nullptr)
	{}

	SDLWindow::~SDLWindow()
	{
		m_pWindow = NULL;
	}

	void SDLWindow::HandleAllEvents(SDL_Event& event)
	{
		if (HandleInputEvents(event)) return;
		switch (event.type)
		{
		case SDL_QUIT:
			m_pWindowManager->GetEngine()->RequestQuit();
			return;
		default:
			break;
		}
	}

	void SDLWindow::Open()
	{
		// Create an SDL window that supports Vulkan rendering.
		m_pWindow = SDL_CreateWindow(m_WindowName.c_str(), SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED, m_Width, m_Height, m_WindowFlags);

		if (m_pWindow == NULL) throw new SDLErrorException(SDL_GetError());

		m_pWindowSurface = SDL_GetWindowSurface(m_pWindow);
	}

	void SDLWindow::Close()
	{
		SDL_DestroyWindow(m_pWindow);
		m_pWindow = nullptr;
		m_pWindowSurface = nullptr;

		if (m_pSplashScreen)
		{
			SDL_FreeSurface(m_pSplashScreen);
			m_pSplashScreen = nullptr;
		}
	}

	void SDLWindow::PollEvents()
	{
		SDL_Event event;
		while (PollEvent(&event))
		{
			HandleAllEvents(event);
		}
	}

	void SDLWindow::GetVulkanRequiredExtensions(std::vector<const char*>& extensions)
	{
		uint32_t extensionCount;
		if (!SDL_Vulkan_GetInstanceExtensions(m_pWindow, &extensionCount, NULL))
			throw new GetVulkanExtensionsException();

		size_t currentSize = extensions.size();
		extensions.resize(currentSize + extensionCount);
		if (!SDL_Vulkan_GetInstanceExtensions(m_pWindow, &extensionCount, &extensions[currentSize]))
			throw new GetVulkanExtensionsException();
	}
}
