#pragma once
#include <Versioning.h>

#ifdef GLORY_EDITOR_EXPORTS
// BUILD LIB
#define GLORY_EDITOR_API __declspec(dllexport)
#else
// USE LIB
#define GLORY_EDITOR_API __declspec(dllimport)
#endif

#ifdef GLORY_EDITOR_EXTENSION_EXPORTS
// BUILD LIB
#define GLORY_EDITOR_EXTENSION_API __declspec(dllexport)
#else
// USE LIB
#define GLORY_EDITOR_EXTENSION_API __declspec(dllimport)
#endif

#define MAJOR_VERSION 0
#define MINOR_VERSION 1
#define BUILD_NUMBER 0

namespace Glory::Editor
{
	const Glory::VersionValue VERSION_DATA[] = {
		{"Major", TOSTRING(MAJOR_VERSION)},
		{"Minor", TOSTRING(MINOR_VERSION)},
		{"Build", TOSTRING(BUILD_NUMBER)},
	};

	const Glory::Version Version(VERSION_DATA, 3);
}
