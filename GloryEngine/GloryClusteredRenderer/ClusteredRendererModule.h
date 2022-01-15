#pragma once
#include <RendererModule.h>
#include <Material.h>
#include <MaterialData.h>
#include <FileData.h>

namespace Glory
{
	class ClusteredRendererModule : public RendererModule
	{
	public:
		ClusteredRendererModule();
		virtual ~ClusteredRendererModule();

	private:
		virtual void Initialize() override;
		virtual void Cleanup() override;
		virtual void PostInitialize() override;

		virtual void OnThreadedInitialize() override;
		virtual void OnThreadedCleanup() override;


	private:
		FileData* m_pClusterShaderFile;
		MaterialData* m_pClusterShaderMaterialData;
		Material* m_pClusterShaderMaterial;
	};
}
