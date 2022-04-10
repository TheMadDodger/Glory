#pragma once

#ifdef GLORY_EXPORT_LIB
// BUILD LIB
#define GLORY_EXPORT __declspec(dllexport)
#else
// USE LIB
#define GLORY_EXPORT __declspec(dllimport)
#endif