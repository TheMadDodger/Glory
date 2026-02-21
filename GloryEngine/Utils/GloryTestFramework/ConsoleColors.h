#pragma once

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define CONSOLE_COLOR(color, text) STRINGIFY(\033[1;##color##m##text\033[0m)
#define CONSOLE_COLOR_START(color) STRINGIFY(\033[1;##color##m)
#define CONSOLE_COLOR_END STRINGIFY(\033[0m)

#define COLOR_BLACK 30
#define COLOR_RED 31
#define COLOR_GREEN 32
#define COLOR_YELLOW 33
#define COLOR_BLUE 34
#define COLOR_MAGENTA 35
#define COLOR_CYAN 36
#define COLOR_WHITE 37
#define COLOR_DEFAULT 39

#define CONSOLE_BLACK(text) CONSOLE_COLOR(30, text)
#define CONSOLE_RED(text) CONSOLE_COLOR(31, text)
#define CONSOLE_GREEN(text) CONSOLE_COLOR(32, text)
#define CONSOLE_YELLOW(text) CONSOLE_COLOR(33, text)
#define CONSOLE_BLUE(text) CONSOLE_COLOR(34, text)
#define CONSOLE_MAGENTA(text) CONSOLE_COLOR(35, text)
#define CONSOLE_CYAN(text) CONSOLE_COLOR(36, text)
#define CONSOLE_WHITE(text) CONSOLE_COLOR(37, text)
#define CONSOLE_DEFAULT(text) CONSOLE_COLOR(39, text)

#define CONSOLE_INDENT(x) std::cout << std::setw(x) << " "