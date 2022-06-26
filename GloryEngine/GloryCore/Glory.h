#pragma once

#ifdef GLORY_EXPORTS
// BUILD LIB
#define GLORY_API __declspec(dllexport)
#else
// USE LIB
#define GLORY_API __declspec(dllimport)
#endif

#ifdef GLORY_CORE_EXPORTS
// BUILD LIB
#define GLORY_CORE_API __declspec(dllexport)
#else
// USE LIB
#define GLORY_CORE_API __declspec(dllimport)
#endif