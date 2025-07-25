#pragma once
#include <string>

namespace Glory
{
	class WindowModule;

	struct WindowCreateInfo
	{
		WindowModule* pWindowManager;
		std::string WindowName;
		uint32_t Width;
		uint32_t Height;
		uint32_t WindowFlags;
		bool Fullscreen;
		bool Maximize;
	};

	/**
	 * @brief Window flags
	 * Copied from SDL_video.h
	 */
	enum WindowFlags
	{
		/** @brief Fullscreen window */
		W_Fullscreen = 0x00000001,
		/** @brief Window usable with OpenGL context */
		W_OpenGL = 0x00000002,
		/** @brief Window is visible */
		W_Shown = 0x00000004,
		/** @brief Window is not visible */
		W_Hidden = 0x00000008,
		/** @brief No window decoration */
		W_Borderless = 0x00000010,
		/** @brief Window can be resized */
		W_Resizeable = 0x00000020,
		/** @brief Window is minimized */
		W_Minimized = 0x00000040,
		/** @brief Window is maximized */
		W_Maximized = 0x00000080,
		/** @brief Window has grabbed mouse input */
		W_MouseGrabbed = 0x00000100,
		/** @brief Window has input focus */
		W_InputFocus = 0x00000200,
		/** @brief Window has mouse focus */
		W_MouseFocus = 0x00000400,
		W_FullscreenDesktop = (W_Fullscreen | 0x00001000),
		/** @brief Window should be created in high-DPI mode if supported.
		 * On macOS NSHighResolutionCapable must be set true in the
		 * application's Info.plist for this to have any effect. */
		W_AllowHighDPI = 0x00002000,
		/** @brief Window has mouse captured (unrelated to W_MouseGrabbed) */
		W_MouseCapture = 0x00004000,
		/** @brief Window should always be above others */
		W_AlwaysOnTop = 0x00008000,
		/** @brief Window should not be added to the taskbar */
		W_SkipTaskbar = 0x00010000,
		/** @brief Window should be treated as a utility window */
		W_Utility = 0x00020000,
		/** @brief Window should be treated as a tooltip */
		W_Tooltip = 0x00040000,
		/** @brief Window should be treated as a popup menu */
		W_PopupMenu = 0x00080000,
		/** @brief Window has grabbed keyboard input */
		W_KeyboardGrabbed = 0x00100000,
		/** @brief Window usable for Vulkan surface */
		W_Vulkan = 0x10000000,
		/** @brief Window usable for Metal view */
		W_Metal = 0x20000000,
	};
}
