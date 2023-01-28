#pragma once

#ifdef GLORY_API_EXPORTS
// BUILD LIB
#define GLORY_API_API __declspec(dllexport)
#else
// USE LIB
#define GLORY_API_API __declspec(dllimport)
#endif
