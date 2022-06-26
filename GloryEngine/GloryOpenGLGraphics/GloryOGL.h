#pragma once
#include <Glory.h>
#include <Module.h>
#include <GloryContext.h>

#pragma comment (lib,"opengl32.lib")

extern "C" GLORY_API Glory::Module* LoadModule(Glory::GloryContext * pContext);