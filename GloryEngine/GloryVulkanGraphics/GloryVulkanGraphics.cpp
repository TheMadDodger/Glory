#include "GloryVulkanGraphics.h"
#include "VulkanGraphicsModule.h"

GLORY_API Glory::Module* LoadModule(Glory::GloryContext* pContext)
{
	Glory::GloryContext::SetContext(pContext);
	return new Glory::VulkanGraphicsModule();
}
