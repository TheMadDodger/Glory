#include "GloryASSIMPModelLoader.h"
#include "ASSIMPModule.h"

GLORY_API Glory::Module* LoadModule()
{
	return new Glory::ASSIMPModule();
}
