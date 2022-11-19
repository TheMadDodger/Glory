#include "GloryMono.h"
#include "GloryMonoScipting.h"

GLORY_API Glory::Module* OnLoadModule(Glory::GloryContext* pContext)
{
	Glory::GloryContext::SetContext(pContext);
	return new Glory::GloryMonoScipting();
}
