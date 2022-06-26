#include "GloryClusteredRenderer.h"
#include "ClusteredRendererModule.h"

GLORY_API Glory::Module* LoadModule()
{
	return new Glory::ClusteredRendererModule();
}
