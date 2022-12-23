#include <GloryContext.h>
#include "GainputModule.h"
#include "GloryGainput.h"

GLORY_API Glory::Module* OnLoadModule(Glory::GloryContext* pContext)
{
    Glory::GloryContext::SetContext(pContext);
    return new Glory::GainputModule();
}
