#include "ClusteredRendererModule.h"
#include <Engine.h>
#include <GraphicsModule.h>
#include <GPUResourceManager.h>
#include <FileLoaderModule.h>

namespace Glory
{
	ClusteredRendererModule::ClusteredRendererModule() : m_pClusterShader(nullptr)
	{
	}

	ClusteredRendererModule::~ClusteredRendererModule()
	{
	}

	void ClusteredRendererModule::Initialize()
	{
		FileImportSettings importSettings;
		importSettings.Flags = (int)(std::ios::ate | std::ios::binary);
		m_pClusterShaderFile = (FileData*)m_pEngine->GetLoaderModule<FileData>()->Load("./Models/monkeh.fbx", importSettings);

		std::vector<FileData*> pShaderFiles = { m_pClusterShaderFile };
		std::vector<ShaderType> shaderTypes = { ShaderType::ST_Vertex, ShaderType::ST_Fragment };

		m_pClusterShaderMaterialData = new MaterialData({ m_pClusterShaderFile }, { ShaderType::ST_Compute });
	}

	void ClusteredRendererModule::Cleanup()
	{
		delete m_pClusterShaderFile;
		m_pClusterShaderFile = nullptr;

		delete m_pClusterShaderMaterialData;
		m_pClusterShaderMaterialData = nullptr;
	}

	void ClusteredRendererModule::PostInitialize()
	{
	}

	void ClusteredRendererModule::OnThreadedInitialize()
	{
		m_pClusterShaderMaterial = m_pEngine->GetGraphicsModule()->GetResourceManager()->CreateMaterial(m_pClusterShaderMaterialData);
	}

	void ClusteredRendererModule::OnThreadedCleanup()
	{

	}
}