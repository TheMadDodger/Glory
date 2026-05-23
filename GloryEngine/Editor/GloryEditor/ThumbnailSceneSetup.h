#pragma once
#include <UUID.h>
#include <Entity.h>

#include <GraphicsHandles.h>

namespace Glory
{
	class GScene;
	class Resource;
	class GraphicsDevice;
	class Renderer;
}

namespace Glory::Editor
{
	void SetupMaterialScene(Entity root, UUID materialID);
	bool CanRenderMaterial(UUID materialID);

	void SetupMeshScene(Entity root, UUID meshID);
	bool CanRenderMesh(UUID meshID);

	void SetupImageScene(Entity root, UUID imageID);
	bool CanRenderImage(UUID imageID);
	void CustomRenderImage(UUID imageID, GraphicsDevice* pDevice, Renderer* pRenderer,
		uint32_t frameIndex, CommandBufferHandle commandBuffer);
}