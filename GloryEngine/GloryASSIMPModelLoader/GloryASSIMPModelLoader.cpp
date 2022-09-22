#include "GloryASSIMPModelLoader.h"
#include "ASSIMPModule.h"

GLORY_API Glory::Module* LoadModule(Glory::GloryContext* pContext)
{
	Glory::GloryContext::SetContext(pContext);
	return new Glory::ASSIMPModule();
}
