#pragma once
#include "Versioning.h"

#ifdef GLORY_EXPORTS
// BUILD LIB
#define GLORY_API __declspec(dllexport)
#else
// USE LIB
#define GLORY_API __declspec(dllimport)
#endif

#define CORE_VERSION_MAJOR 0
#define CORE_VERSION_MINOR 1

const Glory::VersionValue CORE_VERSION_DATA[] = {
	{"Major", TOSTRING(CORE_VERSION_MAJOR)},
	{"Minor", TOSTRING(CORE_VERSION_MINOR)},
};

const Glory::Version CoreVersion(CORE_VERSION_DATA, 2);