#include "SDLWindow.h"
#include "SDLWindowExceptions.h"
#include "WindowModule.h"

#include <Input.h>
#include <RendererModule.h>
#include <Engine.h>
#include <Debug.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_vulkan.h>
#include <iostream>
#include <map>

namespace Glory
{
	const std::map<SDL_Keycode, KeyboardKey> KEYBOARD_KEYMAP = {
		{SDLK_ESCAPE,				KeyEscape,					   },
		{SDLK_F1,					KeyF1,						   },
		{SDLK_F2,					KeyF2,						   },
		{SDLK_F3,					KeyF3,						   },
		{SDLK_F4,					KeyF4,						   },
		{SDLK_F5,					KeyF5,						   },
		{SDLK_F6,					KeyF6,						   },
		{SDLK_F7,					KeyF7,						   },
		{SDLK_F8,					KeyF8,						   },
		{SDLK_F9,					KeyF9,						   },
		{SDLK_F10,					KeyF10,						   },
		{SDLK_F11,					KeyF11,						   },
		{SDLK_F12,					KeyF12,						   },
		{SDLK_F13,					KeyF13,						   },
		{SDLK_F14,					KeyF14,						   },
		{SDLK_F15,					KeyF15,						   },
		{SDLK_F16,					KeyF16,						   },
		{SDLK_F17,					KeyF17,						   },
		{SDLK_F18,					KeyF18,						   },
		{SDLK_F19,					KeyF19,						   },
		{SDLK_PRINTSCREEN,			KeyPrint,					   },
		{SDLK_SCROLLLOCK,			KeyScrollLock,				   },
		//{SDLK_BREAK,				KeyBreak,					   },

		{SDLK_SPACE,				KeySpace,					   },

		//{SDLK_APOSTROPHE,			KeyApostrophe,				   },
		{SDLK_COMMA,				KeyComma,					   },
		{SDLK_MINUS,				KeyMinus,					   },
		{SDLK_PERIOD,				KeyPeriod,					   },
		{SDLK_SLASH,				KeySlash,					   },

		{SDLK_0,					Key0,						   },
		{SDLK_1,					Key1,						   },
		{SDLK_2,					Key2,						   },
		{SDLK_3,					Key3,						   },
		{SDLK_4,					Key4,						   },
		{SDLK_5,					Key5,						   },
		{SDLK_6,					Key6,						   },
		{SDLK_7,					Key7,						   },
		{SDLK_8,					Key8,						   },
		{SDLK_9,					Key9,						   },

		{SDLK_SEMICOLON,			KeySemicolon,				   },
		{SDLK_LESS,					KeyLess,						},
		{SDLK_EQUALS,				KeyEqual,					   },

		{SDLK_a,					KeyA,						   },
		{SDLK_b,					KeyB,						   },
		{SDLK_c,					KeyC,						   },
		{SDLK_d,					KeyD,						   },
		{SDLK_e,					KeyE,						   },
		{SDLK_f,					KeyF,						   },
		{SDLK_g,					KeyG,						   },
		{SDLK_h,					KeyH,						   },
		{SDLK_i,					KeyI,						   },
		{SDLK_j,					KeyJ,						   },
		{SDLK_k,					KeyK,						   },
		{SDLK_l,					KeyL,						   },
		{SDLK_m,					KeyM,						   },
		{SDLK_n,					KeyN,						   },
		{SDLK_o,					KeyO,						   },
		{SDLK_p,					KeyP,						   },
		{SDLK_q,					KeyQ,						   },
		{SDLK_r,					KeyR,						   },
		{SDLK_s,					KeyS,						   },
		{SDLK_t,					KeyT,						   },
		{SDLK_u,					KeyU,						   },
		{SDLK_v,					KeyV,						   },
		{SDLK_w,					KeyW,						   },
		{SDLK_x,					KeyX,						   },
		{SDLK_y,					KeyY,						   },
		{SDLK_z,					KeyZ,						   },

		{SDLK_LEFTBRACKET,			KeyBracketLeft,					},
		{SDLK_BACKSLASH,			KeyBackslash,					},
		{SDLK_RIGHTBRACKET,			KeyBracketRight,				},

		//{SDLK_GRAVE,				KeyGrave,					   },

		{SDLK_LEFT,					KeyLeft,						   },
		{SDLK_RIGHT,				KeyRight,					   },
		{SDLK_UP,					KeyUp,						   },
		{SDLK_DOWN,					KeyDown,						   },
		{SDLK_INSERT,				KeyInsert,					   },
		{SDLK_HOME,					KeyHome,						   },
		{SDLK_DELETE,				KeyDelete,					   },
		{SDLK_END,					KeyEnd,						   },
		{SDLK_PAGEUP,				KeyPageUp,					   },
		{SDLK_PAGEDOWN,				KeyPageDown,					   },

		{SDLK_NUMLOCKCLEAR,			KeyNumLock,					   },
		{SDLK_KP_EQUALS,			KeyKpEqual,					   },
		{SDLK_KP_DIVIDE,			KeyKpDivide,					   },
		{SDLK_KP_MULTIPLY,			KeyKpMultiply,				   },
		{SDLK_KP_MEMSUBTRACT,		KeyKpSubtract,				   },
		{SDLK_KP_MEMADD,			KeyKpAdd,					   },
		{SDLK_KP_ENTER,				KeyKpEnter,					   },
		//{SDLK_KP_INSERT,			KeyKpInsert,					   },
		//{SDLK_KP_END,				KeyKpEnd,					   },
		//{SDLK_KP_DOWN,			KeyKpDown,					   },
		//{SDLK_KpPageDown,			KeyKpPageDown,				   },
		//{SDLK_KpLeft,				KeyKpLeft,					   },
		//{SDLK_KpBegin,			KeyKpBegin,					   },
		//{SDLK_KpRight,			KeyKpRight,					   },
		//{SDLK_KpHome,				KeyKpHome,					   },
		//{SDLK_KpUp,				KeyKpUp,						   },
		//{SDLK_KpPageUp,			KeyKpPageUp,					   },
		//{SDLK_KpDelete,			KeyKpDelete,					   },

		{SDLK_BACKSPACE,			KeyBackSpace,				   },
		{SDLK_TAB,					KeyTab,						   },
		{SDLK_RETURN,				KeyReturn,					   },
		{SDLK_CAPSLOCK,				KeyCapsLock,					   },
		{SDLK_LSHIFT,				KeyShiftL,					   },
		{SDLK_LCTRL,				KeyCtrlL,					   },
		//{SDLK_SUPER,				KeySuperL,					   },
		{SDLK_LALT,					KeyAltL,						   },
		{SDLK_RALT,					KeyAltR,						   },
		//{SDLK_SuperR,				KeySuperR,					   },
		{SDLK_MENU,					KeyMenu,						   },
		{SDLK_RCTRL,				KeyCtrlR,					   },
		{SDLK_RSHIFT,				KeyKpMultiply,					   },

		//{SDLK_BACK,				KeyBack,						   },
		//{SDLK_SoftLeft,			KeySoftLeft,					   },
		//{SDLK_SoftRight,			KeySoftRight,				   },
		//{SDLK_Call,				KeyCall,						   },
		//{SDLK_Endcall,			KeyEndcall,					   },
		//{SDLK_Star,				KeyStar,						   },
		//{SDLK_Pound,				KeyPound,					   },
		//{SDLK_DpadCenter,			KeyDpadCenter,				   },
		{SDLK_VOLUMEUP,				KeyVolumeUp,					   },
		{SDLK_VOLUMEDOWN,			KeyVolumeDown,				   },
		{SDLK_POWER,				KeyPower,					   },
		//{SDLK_Camera,				KeyCamera,					   },
		{SDLK_CLEAR,				KeyClear,					   },
		//{SDLK_Symbol,				KeySymbol,					   },
		//{SDLK_Explorer,			KeyExplorer,					   },
		//{SDLK_Envelope,			KeyEnvelope,					   },
		{SDLK_EQUALS,				KeyEquals,					   },
		{SDLK_AT,					KeyAt,						   },
		//{SDLK_Headsethook,		KeyHeadsethook,				   },
		//{SDLK_Focus,				KeyFocus,					   },
		{SDLK_PLUS,					KeyPlus,						   },
		//{SDLK_Notification,		KeyNotification,				   },
		//{SDLK_Search,				KeySearch,					   },
		//{SDLK_MediaPlayPause,		KeyMediaPlayPause,			   },
		//{SDLK_MediaStop,			KeyMediaStop,				   },
		//{SDLK_MediaNext,			KeyMediaNext,				   },
		//{SDLK_MediaPrevious,		KeyMediaPrevious,			   },
		//{SDLK_MediaRewind,		KeyMediaRewind,				   },
		//{SDLK_MediaFastForward,	KeyMediaFastForward,			   },
		//{SDLK_Mute,				KeyMute,						   },
		//{SDLK_Pictsymbols,		KeyPictsymbols,				   },
		//{SDLK_SwitchCharset,		KeySwitchCharset,			   },

		{SDL_SCANCODE_AC_FORWARD,		   KeyForward,					   },
		//{SDLK_Extra1,				KeyExtra1,					   },
		//{SDLK_Extra2,				KeyExtra2,					   },
		//{SDLK_Extra3,				KeyExtra3,					   },
		//{SDLK_Extra4,				KeyExtra4,					   },
		//{SDLK_Extra5,				KeyExtra5,					   },
		//{SDLK_Extra6,				KeyExtra6,					   },
		//{SDLK_Fn,					KeyFn,						   },

		//{SDLK_Circumflex,			KeyCircumflex,				   },
		//{SDLK_Ssharp,				KeySsharp,					   },
		//{SDLK_Acute,				KeyAcute,					   },
		{SDLK_ALTERASE,				KeyAltGr,					   },
		//{SDLK_Numbersign,			KeyNumbersign,				   },
		//{SDLK_Udiaeresis,			KeyUdiaeresis,				   },
		//{SDLK_Adiaeresis,			KeyAdiaeresis,				   },
		//{SDLK_Odiaeresis,			KeyOdiaeresis,				   },
		//{SDLK_Section,			KeySection,					   },
		//{SDLK_Aring,				KeyAring,					   },
		//{SDLK_Diaeresis,			KeyDiaeresis,				   },
		//{SDLK_Twosuperior,		KeyTwosuperior,				   },
		//{SDLK_RightParenthesis,	KeyRightParenthesis,			   },
		{SDLK_DOLLAR,				KeyDollar,					   },
		//{SDLK_Ugrave,				KeyUgrave,					   },
		{SDLK_ASTERISK,				KeyAsterisk,							},
		{SDLK_COLON,				KeyColon,					   },
		{SDLK_EXCLAIM,				KeyExclam,					   },

		{SDLK_KP_LEFTBRACE,			KeyBraceLeft,				   },
		{SDLK_KP_RIGHTBRACE,		KeyBraceRight,				},
		{SDLK_SYSREQ,				KeySysRq,					   },
		{SDLK_BACKQUOTE,			KeyBackQuote,					},

		{SDLK_KP_MINUS,				KeyKpMinus						},
		{SDLK_KP_PLUS,				KeyKpPlus						},
		{SDLK_KP_1,					KeyKp1							},
		{SDLK_KP_2,					KeyKp2							},
		{SDLK_KP_3,					KeyKp3							},
		{SDLK_KP_4,					KeyKp4							},
		{SDLK_KP_5,					KeyKp5							},
		{SDLK_KP_6,					KeyKp6							},
		{SDLK_KP_7,					KeyKp7							},
		{SDLK_KP_8,					KeyKp8							},
		{SDLK_KP_9,					KeyKp9							},
		{SDLK_KP_0,					KeyKp0							},
		{SDLK_KP_PERIOD,			KeyKpPeriod						},
	};
	
	const std::map<SDL_Keymod, KeyboardMod> KEYBOARD_MODMAP =		{
		{SDL_Keymod::KMOD_ALT,		ModAlt							},
		{SDL_Keymod::KMOD_LALT,		ModLAlt							},
		{SDL_Keymod::KMOD_RALT,		ModRAlt							},
		{SDL_Keymod::KMOD_SHIFT,	ModShift						},
		{SDL_Keymod::KMOD_LSHIFT,	ModLShift						},
		{SDL_Keymod::KMOD_RSHIFT,	ModRShift						},
		{SDL_Keymod::KMOD_CTRL,		ModCtrl							},
		{SDL_Keymod::KMOD_RCTRL,	ModRCtrl						},
		{SDL_Keymod::KMOD_LCTRL,	ModLCtrl						},
		{SDL_Keymod::KMOD_CAPS,		ModCaps							},
		{SDL_Keymod::KMOD_GUI,		ModGui							},
		{SDL_Keymod::KMOD_LGUI,		ModLGui							},
		{SDL_Keymod::KMOD_RGUI,		ModRGui							},
		{SDL_Keymod::KMOD_NUM,		ModNum							},
		{SDL_Keymod::KMOD_MODE,		ModMode							},
		{SDL_Keymod::KMOD_SCROLL,	ModScroll						},
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
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
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
			if (iter->second == KeyboardKey::KeyAltL) m_LAltDown = true;
			if (iter->second == KeyboardKey::KeyAltR) m_RAltDown = true;

			if ((m_LAltDown || m_RAltDown) && iter->second == KeyboardKey::KeyReturn)
				SetFullscreen(!m_Fullscreen, false);

			inputEvent.InputDeviceType = InputDeviceType::Keyboard;
			inputEvent.KeyID = iter->second;
			inputEvent.State = InputState::KeyDown;
			inputEvent.SourceDeviceID = 0;
			inputEvent.Value = 1.0f;
			inputEvent.Delta = 1.0f;
			inputEvent.KeyMods = KeyboardMod(event.key.keysym.mod);
			return ForwardInputEvent(inputEvent);
		}
		case SDL_KEYUP:
		{
			SDL_Keycode keycode = event.key.keysym.sym;
			const auto iter = KEYBOARD_KEYMAP.find(keycode);
			if (iter == KEYBOARD_KEYMAP.end()) break;
			if (iter->second == KeyboardKey::KeyAltL) m_LAltDown = false;
			if (iter->second == KeyboardKey::KeyAltR) m_RAltDown = false;

			inputEvent.InputDeviceType = InputDeviceType::Keyboard;
			inputEvent.KeyID = iter->second;
			inputEvent.State = InputState::KeyUp;
			inputEvent.SourceDeviceID = 0;
			inputEvent.Value = 0.0f;
			inputEvent.Delta = -1.0f;
			inputEvent.KeyMods = KeyboardMod(event.key.keysym.mod);
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
			inputEvent.KeyMods = KeyboardMod::ModNone;
			const bool consumed = ForwardInputEvent(inputEvent);

			if (inputEvent.KeyID == MouseButton::MouseButtonLeft)
			{
				CursorEvent cursorEvent;
				cursorEvent.Type = CursorEvent::Button;
				cursorEvent.InputDeviceType = InputDeviceType::Mouse;
				cursorEvent.SourceDeviceID = inputEvent.SourceDeviceID;
				cursorEvent.Cursor = glm::vec2{ 0.0f, 0.0f };
				cursorEvent.IsDelta = false;
				cursorEvent.IsDown = true;
				ForwardCursorEvent(cursorEvent);
			}
			return consumed;
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
			inputEvent.KeyMods = KeyboardMod::ModNone;
			const bool consumed = ForwardInputEvent(inputEvent);

			if (inputEvent.KeyID == MouseButton::MouseButtonLeft)
			{
				CursorEvent cursorEvent;
				cursorEvent.Type = CursorEvent::Button;
				cursorEvent.InputDeviceType = InputDeviceType::Mouse;
				cursorEvent.SourceDeviceID = inputEvent.SourceDeviceID;
				cursorEvent.Cursor = glm::vec2{ 0.0f, 0.0f };
				cursorEvent.IsDelta = false;
				cursorEvent.IsDown = false;
				ForwardCursorEvent(cursorEvent);
			}
			return consumed;
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
				inputEvent.KeyMods = KeyboardMod::ModNone;
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
				inputEvent.KeyMods = KeyboardMod::ModNone;
				consumed |= ForwardInputEvent(inputEvent);

				/* Button event */
				inputEvent.InputDeviceType = InputDeviceType::Mouse;
				inputEvent.KeyID = event.wheel.y < 0 ? MouseButton::MouseButtonWheelDown : MouseButton::MouseButtonWheelUp;
				inputEvent.State = InputState::KeyDown;
				inputEvent.SourceDeviceID = event.wheel.which;
				inputEvent.Value = 1.0f;
				inputEvent.Delta = 1.0f;
				inputEvent.KeyMods = KeyboardMod::ModNone;
				consumed |= ForwardInputEvent(inputEvent);
				inputEvent.State = InputState::KeyUp;
				inputEvent.Value = 0.0f;
				inputEvent.Delta = -1.0f;
				consumed |= ForwardInputEvent(inputEvent);

				CursorEvent cursorEvent;
				cursorEvent.Type = CursorEvent::Scroll;
				cursorEvent.InputDeviceType = InputDeviceType::Mouse;
				cursorEvent.SourceDeviceID = event.motion.which;
				cursorEvent.Cursor = glm::vec2{ float(event.wheel.x), float(event.wheel.y) };
				cursorEvent.IsDelta = true;
				cursorEvent.IsDown = false;
				ForwardCursorEvent(cursorEvent);
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
			inputEvent.KeyMods = KeyboardMod::ModNone;
			consumed |= ForwardInputEvent(inputEvent);

			inputEvent.InputDeviceType = InputDeviceType::Mouse;
			inputEvent.KeyID = MouseAxis::MouseAxisY;
			inputEvent.State = InputState::Axis;
			inputEvent.SourceDeviceID = event.motion.which;
			inputEvent.Value = (float)event.motion.y;
			inputEvent.Delta = (float)event.motion.yrel;
			consumed |= ForwardInputEvent(inputEvent);

			CursorEvent cursorEvent;
			cursorEvent.Type = CursorEvent::Motion;
			cursorEvent.InputDeviceType = InputDeviceType::Mouse;
			cursorEvent.SourceDeviceID = event.motion.which;
			cursorEvent.Cursor = glm::vec2{ event.motion.x, event.motion.y };
			cursorEvent.IsDelta = false;
			cursorEvent.IsDown = false;
			ForwardCursorEvent(cursorEvent);
			return consumed;
		}
		case SDL_TEXTINPUT:
		{
			TextEvent text;
			text.InputDeviceType = InputDeviceType::Keyboard;
			text.Character = event.text.text[0];
			return ForwardTextEvent(text);
		}
		default:
			// Do nothing.
			break;
		}

		return false;
	}

	void SDLWindow::HandleWindowFocusEvents(SDL_WindowEvent& event)
	{
		switch (event.event)
		{
		case SDL_WINDOWEVENT_SHOWN:
			/* Window has been shown */
			m_IsShown = true;
			break;
		case SDL_WINDOWEVENT_HIDDEN:
			/* Window has been hidden */
			m_IsShown = false;
			break;
		case SDL_WINDOWEVENT_ENTER:
			/* Window has gained mouse focus */
			break;
		case SDL_WINDOWEVENT_LEAVE:
			/* Window has lost mouse focus */
			break;
		case SDL_WINDOWEVENT_FOCUS_GAINED:
			/* Window has gained keyboard focus */
			m_HasFocus = true;
			break;
		case SDL_WINDOWEVENT_FOCUS_LOST:
			/* Window has lost keyboard focus */
			m_HasFocus = false;
			break;
		case SDL_WINDOWEVENT_TAKE_FOCUS:
			/* Window is being offered a focus (should SetWindowInputFocus() on itself or a subwindow, or ignore) */
			break;
		default:
			break;
		}
	}

	void SDLWindow::HandleWindowSizeEvents(SDL_WindowEvent& event)
	{
		Engine* pEngine = m_pWindowManager->GetEngine();
		RendererModule* pRenderer = pEngine->GetMainModule<RendererModule>();
		if (!pRenderer) return;
		switch (event.event)
		{
		case SDL_WINDOWEVENT_EXPOSED:
			/* Window has been exposed and should be redrawn */
			break;
		case SDL_WINDOWEVENT_MOVED:
			/* Window has been moved to data1, data2 */
			break;
		case SDL_WINDOWEVENT_RESIZED:
			/* Window has been resized to data1xdata2 */
			pRenderer->OnWindowResize({ event.data1, event.data2 });
			break;
		case SDL_WINDOWEVENT_SIZE_CHANGED:
			/* The window size has changed, either as a result of an API call or through the system or user changing the window size. */
			break;
		case SDL_WINDOWEVENT_MINIMIZED:
			/* Window has been minimized */
			m_Maximized = false;
			pRenderer->SetEnabled(false);
			break;
		case SDL_WINDOWEVENT_MAXIMIZED:
			/* Window has been maximized */
			m_Maximized = true;
			pRenderer->SetEnabled(true);
			break;
		case SDL_WINDOWEVENT_RESTORED:
			/* Window has been restored to normal size and position */
			m_Maximized = false;
			pRenderer->SetEnabled(true);
			break;
		case SDL_WINDOWEVENT_ICCPROF_CHANGED:
			/* The ICC profile of the window's display has changed. */
			break;
		case SDL_WINDOWEVENT_DISPLAY_CHANGED:
			/* Window has been moved to display data1. */
			break;

		default:
			break;
		}
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

	void SDLWindow::SetCursorPosition(int x, int y)
	{
		if (m_ForceUnlockCursor || !m_HasFocus || !m_IsShown) return;
		SDL_WarpMouseInWindow(m_pWindow, x, y);
	}

	void SDLWindow::SetFullscreen(bool fullscreen, bool borderless)
	{
		m_Fullscreen = fullscreen;
		SDL_SetWindowFullscreen(m_pWindow, !fullscreen ? 0 :
			(borderless ? SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_FULLSCREEN));
	}

	void SDLWindow::Maximize()
	{
		SDL_MaximizeWindow(m_pWindow);
	}

	void SDLWindow::SetGLSwapInterval(int interval)
	{
		if (SDL_GL_SetSwapInterval(interval) < 0)
		{
			std::cerr << "Could not set SDL GL Swap interval: " << SDL_GetError() << std::endl;
			return;
		}
	}

	void SDLWindow::UpdateCursorShow()
	{
		const bool show = m_ForceShowCursor || m_ShowCursor;
		SDL_ShowCursor(show ? SDL_ENABLE : SDL_DISABLE);
	}

	void SDLWindow::UpdateGrabInput()
	{
		const bool grab = !m_ForceUngrabInput && m_GrabInput;
		SDL_SetRelativeMouseMode(grab ? SDL_TRUE : SDL_FALSE);
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

	std::string SDLWindow::GetClipboardText()
	{
		char* clipboard = SDL_GetClipboardText();
		std::string clipboardStr = clipboard;
		SDL_free(clipboard);
		return clipboardStr;
	}

	void SDLWindow::StartTextInput()
	{
		SDL_StartTextInput();
	}

	void SDLWindow::StopTextInput()
	{
		SDL_StopTextInput();
	}

	SDLWindow::SDLWindow(const WindowCreateInfo& createInfo, WindowModule* pWindowManager) : Window(createInfo, pWindowManager),
		m_pWindow(nullptr), m_GLSDLContext(NULL), m_pWindowSurface(nullptr), m_pSplashScreen(nullptr), m_LAltDown(false), m_RAltDown(false)
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
		case SDL_WINDOWEVENT:
			HandleWindowFocusEvents(event.window);
			HandleWindowSizeEvents(event.window);
			break;
		default:
			break;
		}
	}

	void SDLWindow::Open()
	{
		if (m_Width == 0 && m_Height == 0)
			m_pWindowManager->GetCurrentScreenResolution(m_Width, m_Height);

		// Create an SDL window that supports Vulkan rendering.
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
		m_pWindow = SDL_CreateWindow(m_WindowName.c_str(), SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED, m_Width, m_Height, m_WindowFlags);
		SetFullscreen(m_Fullscreen, false);
		if (m_Maximized) Maximize();

		if (m_pWindow == NULL)
			m_pWindowManager->GetEngine()->GetDebug().LogFatalError(SDL_GetError());

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
