#pragma once

#ifdef GLORY_RUNTIME_APPLICATION_EXPORTS
// BUILD LIB
#define GLORY_RUNTIME_APPLICATION_API __declspec(dllexport)
#else
// USE LIB
#define GLORY_RUNTIME_APPLICATION_API __declspec(dllimport)
#endif
