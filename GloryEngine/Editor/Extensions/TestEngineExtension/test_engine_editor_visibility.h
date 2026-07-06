#pragma once

#ifdef GLORY_TESTENGINE_EXTENSION_EXPORTS
// BUILD LIB
#define GLORY_TESTENGINE_EXTENSION_API __declspec(dllexport)
#else
// USE LIB
#define GLORY_TESTENGINE_EXTENSION_API __declspec(dllimport)
#endif
