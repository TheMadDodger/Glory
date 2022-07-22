#pragma once

#ifdef GLORY_EDITOR_EXPORTS
// BUILD LIB
#define GLORY_EDITOR_API __declspec(dllexport)
#else
// USE LIB
#define GLORY_EDITOR_API __declspec(dllimport)
#endif
