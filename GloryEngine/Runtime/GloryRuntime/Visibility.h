#pragma once

#ifdef GLORY_RUNTIME_EXPORTS
// BUILD LIB
#define GLORY_RUNTIME_API __declspec(dllexport)
#else
// USE LIB
#define GLORY_RUNTIME_API __declspec(dllimport)
#endif
