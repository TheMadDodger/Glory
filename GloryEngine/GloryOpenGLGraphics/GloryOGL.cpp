#include "GloryOGL.h"
#include "OpenGLGraphicsModule.h"

GLORY_API Glory::Module* LoadModule(Glory::GloryContext* pContext)
{
	Glory::GloryContext::SetContext(pContext);
	return new Glory::OpenGLGraphicsModule();
}