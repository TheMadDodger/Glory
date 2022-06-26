#include "GloryVulkanGraphics.h"
#include "VulkanGraphicsModule.h"

GLORY_API Glory::Module* LoadModule()
{
	return new Glory::VulkanGraphicsModule();
}
