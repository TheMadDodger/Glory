#include "GloryOGL.h"
#include "OpenGLGraphicsModule.h"

GLORY_API Glory::Module* LoadModule()
{
	return new Glory::OpenGLGraphicsModule();
}