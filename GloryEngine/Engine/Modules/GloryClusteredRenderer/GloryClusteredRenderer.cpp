#include "GloryClusteredRenderer.h"
#include "ClusteredRendererModule.h"

GLORY_API Glory::Module* OnLoadModule(Glory::GloryContext* pContext)
{
	Glory::GloryContext::SetContext(pContext);
	return new Glory::ClusteredRendererModule();
}
