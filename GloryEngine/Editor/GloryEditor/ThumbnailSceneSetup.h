#pragma once
#include <UUID.h>
#include <Entity.h>

namespace Glory
{
	class GScene;
	class Resource;
}

namespace Glory::Editor
{
	void SetupMaterialScene(Entity root, UUID materialID);
}