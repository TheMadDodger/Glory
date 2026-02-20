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
	bool CanRenderMaterial(UUID materialID);

	void SetupMeshScene(Entity root, UUID meshID);
	bool CanRenderMesh(UUID meshID);
}